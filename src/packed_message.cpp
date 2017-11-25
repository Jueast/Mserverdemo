#include "packed_message.hpp"
void build_packed_message(PackedMessage& pm, int id, Request::RequestType value, std::string s){
    Request r;
    r.set_id(id);
    r.set_type(value);
    r.set_content(s);
    pm.encode_request(r);
}

size_t PackedMessage::decode_header(){
    body_length_ = *(reinterpret_cast<uint32_t*>(data_.data()));
    return body_length_;
}
void PackedMessage::encode_request(const Request & r){
    std::string s;
    r.SerializeToString(&s);
    body_length_ = s.length();
    auto p = reinterpret_cast<char*>(&body_length_);
    std::copy(p, p + header_length, data_.begin());
    std::copy(s.begin(), s.end(), data_.begin() + header_length);
}

Request PackedMessage::decode_request(){
    Request r;
    decode_header();
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
