#include <packed_message.hpp>

size_t PackedMessage::decode_header(){
    std::sscanf(data_.data(), "%4zu", &body_length_);
    return body_length_;
}
void PackedMessage::encode_request(const Request & r){
    std::string s;
    r.SerializeToString(&s);
    body_length_ = s.length();
    auto p = reinterpret_cast<char*>(&body_length_);
    std::copy(p, p+4, data_.begin());
    std::copy(s.begin(), s.end(), data_.begin() + 4);
}

Request PackedMessage::decode_request(){
    Request r;
    std::sscanf(data_.data(), "%4zu", &body_length_);
    bool flag = r.ParseFromString(std::string(data_.begin() + header_length, 
                                  data_.end() + header_length + body_length_));
    return r;
}

#ifdef PACKED_MESSAGE_TEST
int main(){
    PackedMessage pm;
    Request o,r;
    o.set_type(Request::LOGIN);
    o.set_id(1);
    o.set_content(std::string("kkk"));

    pm.encode_request(o);
    std::cout << std::string(pm.data().begin(), pm.data().begin()+pm.body_length()) << std::endl;
    std::cout << pm.body_length() << std::endl;
    r = pm.decode_request();
    std::cout << r.type() << std::endl;
    std::cout << r.id() << std::endl;
    std::cout << r.content() << std::endl;
    return 0;
}
#endif
