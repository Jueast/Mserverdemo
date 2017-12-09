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

