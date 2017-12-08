#include "mpack_message.hpp"
namespace MNet{

uint32_t MpackMessage::decode_header(){
    body_length_ = *(reinterpret_cast<uint32_t*>(data_));
    return body_length_;
}

bool MpackMessage::encode_mpack(const Mpack& r){
    std::string s;
    r.SerializeToString(&s);
    body_length_ = s.length();
    if(body_length_ > max_body_length)
        return false;
    auto p = reinterpret_cast<char*>(&body_length_);
    std::copy(p, p + header_length, data_);
    std::copy(s.begin(), s.end(), data_ + header_length);
    return true;
}
Mpack MpackMessage::decode_mpack(){
    Mpack::Mpack r;
    decode_header();
    bool flag = r.ParseFromString(std::string(data_ + header_length, 
                                  data_ + header_length + body_length_));
    return r;
}


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
