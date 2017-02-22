#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace boost::asio;
io_service service;


class toClient :
	public boost::enable_shared_from_this<toClient> {
public:
	typedef boost::shared_ptr<toClient> ptr;
	typedef boost::system::error_code error_code;
	static ptr newConnect() {
		ptr newest(new toClient);
		return newest;
	}
	void start();
	ip::tcp::socket &sock();
	void do_read();
	size_t read_complete(const error_code &err, size_t bytes);
	void on_read(const error_code &err, size_t bytes);
	void do_write(const std::string &msg);
	void on_write(const error_code &err, size_t bytes);
	void stop();
private:
	toClient() : started_(false), sock_(service) {}
	char readBuf_[1024];
	char writeBuf_[1024];
	bool started_;
	bool registrated_;
	bool signedIn_;
	ip::tcp::socket sock_;
};
ip::tcp::socket &toClient::sock() {
	return sock_;
}
void toClient::start() {
	started_ = true;
	do_read();
}
void toClient::do_read() {
	async_read(sock_, buffer(readBuf_),  boost::bind(&toClient::read_complete, shared_from_this(), _1, _2),
						boost::bind(&toClient::on_read, shared_from_this(), _1, _2));	
}
size_t toClient::read_complete(const error_code &err, size_t bytes) {
	if (err)
		return 1;
	bool isComplete = std::find(readBuf_, readBuf_ + bytes, '\n') < readBuf_ + bytes;
	return isComplete ? 0 : 1;
}
void toClient::on_read(const error_code &err, size_t bytes) {
	if (err)
		stop();
	std::string answer(readBuf_, bytes);
	do_write(answer);
}
void toClient::do_write(const std::string &msg) {
	std::copy(msg.begin(), msg.end(), writeBuf_);
	sock_.async_write_some(buffer(writeBuf_, msg.size()), boost::bind(&toClient::on_write, shared_from_this(), _1, _2));
}
void toClient::on_write(const error_code &err, size_t bytes) {
	do_read();
}
void toClient::stop() {
	if (!started_)
		return;
	started_ = false;
	sock_.close();
}
ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8000));
void handle_connection(toClient::ptr client, const toClient::error_code &err) {
	client->start();
	toClient::ptr newClient = toClient::newConnect();
	acceptor.async_accept(newClient->sock(), boost::bind(handle_connection, newClient, _1));
}
int main() {
	toClient::ptr client = toClient::newConnect();
	acceptor.async_accept(client->sock(), boost::bind(handle_connection, client, _1));
	service.run();	
	return 0;
}
