#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace boost::asio;
io_service service;

class toServer :
	public boost::enable_shared_from_this<toServer> {
public:
	typedef boost::shared_ptr<toServer> ptr;
	typedef boost::system::error_code error_code;
	static ptr newConnect(ip::tcp::endpoint ep) {
		ptr newest(new toServer);
		newest->connect(ep);
		return newest;
	}
	void connect(ip::tcp::endpoint ep);
	void on_connect(const error_code &err);
	void do_write(const std::string msg);
	void on_write(const error_code &err, size_t bytes);
	void do_read();
	size_t read_complete(const error_code &err, size_t bytes);
	void on_read(const error_code &err, size_t bytes);
	void sendMessage();
	void stop();
private:
	toServer() : started_(true), sock_(service) {}
	char readBuf_[1024];
	char writeBuf_[1024];
	bool started_;
	ip::tcp::socket sock_;
};
void toServer::connect(ip::tcp::endpoint ep) {
	sock_.async_connect(ep, boost::bind(&toServer::on_connect, shared_from_this(), _1));
	started_ = true;
}
void toServer::on_connect(const error_code &err) {
	if (err)
		stop();
	std::string message;
	getline(std::cin, message);
	do_write(message + '\n');
}
void toServer::do_write(const std::string msg) {
	std::copy(msg.begin(), msg.end(), writeBuf_);
	sock_.async_write_some(buffer(writeBuf_, msg.size()), boost::bind(&toServer::on_write, shared_from_this(), _1, _2));
}
void toServer::on_write(const error_code &err, size_t bytes) {
	if (err)
		stop();
	do_read();
}
void toServer::do_read() {
	async_read(sock_, buffer(readBuf_), boost::bind(&toServer::read_complete, shared_from_this(), _1, _2),
						boost::bind(&toServer::on_read, shared_from_this(), _1, _2));
}
size_t toServer::read_complete(const error_code &err, size_t bytes) {
	if (err)
		return 1;
	bool isComplete = std::find(readBuf_, readBuf_ + bytes, '\n') < readBuf_ + bytes;
	return isComplete ? 0 : 1;
}
void toServer::on_read(const error_code &err, size_t bytes) {
	if (err)
		stop();
	std::string answer(readBuf_, bytes - 1);
	std::cout << "Server answered: " << answer << std::endl;
	std::string message;
	std::getline(std::cin, message);
	do_write(message + '\n');
}
/*void toServer::sendMessage() {
	std::string message;
	getline(std::cin, message);
	do_write(message + '\n');
}*/	
void toServer::stop() {
	if (!started_)
		return;
	started_ = false;
	sock_.close();
}
int main() {
	ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8000);
	toServer::ptr newClient = toServer::newConnect(ep);
	service.run();
	return 0;
}
	
