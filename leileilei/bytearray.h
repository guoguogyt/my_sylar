/*
 * @Descripttion: 
 * @version: 
 * @Author: leileilei
 * @Date: 2022-12-20 09:23:57
 * @LastEditors: sueRimn
 * @LastEditTime: 2022-12-29 15:15:40
 */
#pragma once

#include <memory>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

namespace leileilei
{

/**
 * @brief 二进制数组，提供基础类型的序列化，反序列化功能
 */
class ByteArray
{
public:
    typedef std::shared_ptr<ByteArray> ptr;

    /**
     * @brief ByteArray的存储节点 
     */
    struct Node
    {
        /**
         * @brief 构造指定大小的内存块
         * @param s 内存块的字节数
         */
        Node(size_t s);
        // 无参构造函数 
        Node();
        // 析构函数
        ~Node();

        // 内存块指针
        char* ptr;
        // 下一个内存地址
        Node* next;
        // 内存块大小
        size_t size;
    };

    /**
     * @brief 使用指定长度的内存块构造ByteArray
     * @param base_size 内存块大小
     */
    ByteArray(size_t base_size = 4096);
    // 析构函数
    ~ByteArray();

    /**
     * @brief 写入固定长度int_8类型的数据
     * @param value 
        position_ += sizeof(value)
        如果position_ > size_, 则size_ = position_
     */
    void writeFint8(int8_t value);

    /**
     * @brief 写入固定长度uint_8类型的数据
     * @param value 
        position_ += sizeof(value)
        如果position_ > size_, 则size_ = position_
     */
    void writeFuint8(uint8_t value);

    /**
     * @brief 写入固定长度int_16类型的数据
     * @param value 
        position_ += sizeof(value)
        如果position_ > size_, 则size_ = position_
     */
    void writeFint16(int16_t value);

    /**
     * @brief 写入固定长度uint_16类型的数据
     * @param value 
        position_ += sizeof(value)
        如果position_ > size_, 则size_ = position_
     */
    void writeFuint16(uint16_t value);

    /**
     * @brief 写入固定长度int_32类型的数据
     * @param value 
        position_ += sizeof(value)
        如果position_ > size_, 则size_ = position_
     */
    void writeFint32(int32_t value);

    /**
     * @brief 写入固定长度uint_32类型的数据
     * @param value 
        position_ += sizeof(value)
        如果position_ > size_, 则size_ = position_
     */
    void writeFuint32(uint32_t value);

    /**
     * @brief 写入固定长度int_64类型的数据
     * @param value 
        position_ += sizeof(value)
        如果position_ > size_, 则size_ = position_
     */
    void writeFint64(int64_t value);

    /**
     * @brief 写入固定长度uint_64类型的数据
     * @param value 
        position_ += sizeof(value)
        如果position_ > size_, 则size_ = position_
     */
    void writeFuint64(uint64_t value);
    
    /**
     * @brief 写入有符号Varint32类型的数据
     * @post position_ += 实际占用内存(1 ~ 5)
     *       position_ > size_ 则 size_ = position_
     */
    void writeInt32  (int32_t value);
    /**
     * @brief 写入无符号Varint32类型的数据
     * @post position_ += 实际占用内存(1 ~ 5)
     *       position_ > size_ 则 size_ = position_
     */
    void writeUint32 (uint32_t value);

    /**
     * @brief 写入有符号Varint64类型的数据
     * @post position_ += 实际占用内存(1 ~ 10)
     *       position_ > size_ 则 size_ = position_
     */
    void writeInt64  (int64_t value);

    /**
     * @brief 写入无符号Varint64类型的数据
     * @post position_ += 实际占用内存(1 ~ 10)
     *       position_ > size_ 则 size_ = position_
     */
    void writeUint64 (uint64_t value);

    /**
     * @brief 写入float类型的数据
     * @post position_ += sizeof(value)
     *       position_ > size_ 则 size_ = position_
     */
    void writeFloat  (float value);

    /**
     * @brief 写入double类型的数据
     * @post position_ += sizeof(value)
     *       position_ > size_ 则 size_ = position_
     */
    void writeDouble (double value);

    /**
     * @brief 写入std::string类型的数据,用uint16_t作为长度类型
     * @post position_ += 2 + value.size()
     *       position_ > size_ 则 size_ = position_
     */
    void writeStringF16(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,用uint32_t作为长度类型
     * @post position_ += 4 + value.size()
     *       position_ > size_ 则 size_ = position_
     */
    void writeStringF32(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,用uint64_t作为长度类型
     * @post position_ += 8 + value.size()
     *       position_ > size_ 则 size_ = position_
     */
    void writeStringF64(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,用无符号Varint64作为长度类型
     * @post position_ += Varint64长度 + value.size()
     *       position_ > size_ 则 size_ = position_
     */
    void writeStringVint(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,无长度
     * @post position_ += value.size()
     *       如果position_ > m_size 则 m_size = position_
     */
    void writeStringWithoutLength(const std::string& value);

    /**
     * @brief 读取int8_t类型的数据
     * @pre getReadSize() >= sizeof(int8_t)
     * @post position_ += sizeof(int8_t);
     * @exception 如果getReadSize() < sizeof(int8_t) 抛出 std::out_of_range
     */
    int8_t   readFint8();

    /**
     * @brief 读取uint8_t类型的数据
     * @pre getReadSize() >= sizeof(uint8_t)
     * @post position_ += sizeof(uint8_t);
     * @exception 如果getReadSize() < sizeof(uint8_t) 抛出 std::out_of_range
     */
    uint8_t  readFuint8();

    /**
     * @brief 读取int16_t类型的数据
     * @pre getReadSize() >= sizeof(int16_t)
     * @post position_ += sizeof(int16_t);
     * @exception 如果getReadSize() < sizeof(int16_t) 抛出 std::out_of_range
     */
    int16_t  readFint16();

    /**
     * @brief 读取uint16_t类型的数据
     * @pre getReadSize() >= sizeof(uint16_t)
     * @post position_ += sizeof(uint16_t);
     * @exception 如果getReadSize() < sizeof(uint16_t) 抛出 std::out_of_range
     */
    uint16_t readFuint16();

    /**
     * @brief 读取int32_t类型的数据
     * @pre getReadSize() >= sizeof(int32_t)
     * @post position_ += sizeof(int32_t);
     * @exception 如果getReadSize() < sizeof(int32_t) 抛出 std::out_of_range
     */
    int32_t  readFint32();

    /**
     * @brief 读取uint32_t类型的数据
     * @pre getReadSize() >= sizeof(uint32_t)
     * @post position_ += sizeof(uint32_t);
     * @exception 如果getReadSize() < sizeof(uint32_t) 抛出 std::out_of_range
     */
    uint32_t readFuint32();

    /**
     * @brief 读取int64_t类型的数据
     * @pre getReadSize() >= sizeof(int64_t)
     * @post position_ += sizeof(int64_t);
     * @exception 如果getReadSize() < sizeof(int64_t) 抛出 std::out_of_range
     */
    int64_t  readFint64();

    /**
     * @brief 读取uint64_t类型的数据
     * @pre getReadSize() >= sizeof(uint64_t)
     * @post position_ += sizeof(uint64_t);
     * @exception 如果getReadSize() < sizeof(uint64_t) 抛出 std::out_of_range
     */
    uint64_t readFuint64();

    /**
     * @brief 读取有符号Varint32类型的数据
     * @pre getReadSize() >= 有符号Varint32实际占用内存
     * @post position_ += 有符号Varint32实际占用内存
     * @exception 如果getReadSize() < 有符号Varint32实际占用内存 抛出 std::out_of_range
     */
    int32_t  readInt32();

    /**
     * @brief 读取无符号Varint32类型的数据
     * @pre getReadSize() >= 无符号Varint32实际占用内存
     * @post position_ += 无符号Varint32实际占用内存
     * @exception 如果getReadSize() < 无符号Varint32实际占用内存 抛出 std::out_of_range
     */
    uint32_t readUint32();

    /**
     * @brief 读取有符号Varint64类型的数据
     * @pre getReadSize() >= 有符号Varint64实际占用内存
     * @post position_ += 有符号Varint64实际占用内存
     * @exception 如果getReadSize() < 有符号Varint64实际占用内存 抛出 std::out_of_range
     */
    int64_t  readInt64();

    /**
     * @brief 读取无符号Varint64类型的数据
     * @pre getReadSize() >= 无符号Varint64实际占用内存
     * @post position_ += 无符号Varint64实际占用内存
     * @exception 如果getReadSize() < 无符号Varint64实际占用内存 抛出 std::out_of_range
     */
    uint64_t readUint64();

    /**
     * @brief 读取float类型的数据
     * @pre getReadSize() >= sizeof(float)
     * @post position_ += sizeof(float);
     * @exception 如果getReadSize() < sizeof(float) 抛出 std::out_of_range
     */
    float    readFloat();

    /**
     * @brief 读取double类型的数据
     * @pre getReadSize() >= sizeof(double)
     * @post position_ += sizeof(double);
     * @exception 如果getReadSize() < sizeof(double) 抛出 std::out_of_range
     */
    double   readDouble();

    /**
     * @brief 读取std::string类型的数据,用uint16_t作为长度
     * @pre getReadSize() >= sizeof(uint16_t) + size
     * @post position_ += sizeof(uint16_t) + size;
     * @exception 如果getReadSize() < sizeof(uint16_t) + size 抛出 std::out_of_range
     */
    std::string readStringF16();

    /**
     * @brief 读取std::string类型的数据,用uint32_t作为长度
     * @pre getReadSize() >= sizeof(uint32_t) + size
     * @post position_ += sizeof(uint32_t) + size;
     * @exception 如果getReadSize() < sizeof(uint32_t) + size 抛出 std::out_of_range
     */
    std::string readStringF32();

    /**
     * @brief 读取std::string类型的数据,用uint64_t作为长度
     * @pre getReadSize() >= sizeof(uint64_t) + size
     * @post position_ += sizeof(uint64_t) + size;
     * @exception 如果getReadSize() < sizeof(uint64_t) + size 抛出 std::out_of_range
     */
    std::string readStringF64();

    /**
     * @brief 读取std::string类型的数据,用无符号Varint64作为长度
     * @pre getReadSize() >= 无符号Varint64实际大小 + size
     * @post position_ += 无符号Varint64实际大小 + size;
     * @exception 如果getReadSize() < 无符号Varint64实际大小 + size 抛出 std::out_of_range
     */
    std::string readStringVint();

    /**
     * @brief 清空ByteArray 
     * position_ = 0， size_ = 0
     */
    void clear();

    /**
     * @brief 写入size长度的数据 
     * @param buf 内存缓存指针
     * @param size 数据大小
     * position_ += size, 如果position_ > size_ 则 size_ = position_
     */
    void write(const void* buf, size_t size);

    /**
     * @brief 读取size长度的数据 
     * @param buf 内存缓存指针
     * @param size 数据大小
     * position_ += size, 如果position_ > size_ 则 size_ = position_
     * @exception 如果getReadSize() < size 则抛出 std::out_of_range
     */
    void read(void* buf, size_t size);

    /**
     * @brief 读取size长度的数据 
     * @param buf 内存缓存指针
     * @param size 数据大小
     * @param position 读取开始位置
     * @exception 如果 (size_ - position) < size 则抛出 std::out_of_range
     */
    void read(void* buf, size_t size, size_t position) const;

    /**
     * @brief 返回ByteArray当前位置
     * @return size_t 
     */
    size_t getPosition() const {return position_;}

    /**
     * @brief 设置ByetArray当前位置
     * @post 如果position_ > size_ 则 size_ = position_
     * @exception 如果position_ > capacity_ 则抛出 std::out_of_range
     */
    void setPosition(size_t v);

    /**
     * @brief 把ByteArray的数据写入到文件中 
     * @param name 文件名
     * @return true 
     * @return false 
     */
    bool writeToFile(const std::string& name) const;

    /**
     * @brief 从文件中读取数据 
     * @param name 文件名
     * @return true 
     * @return false 
     */
    bool readFromFile(const std::string& name);

    /**
     * @brief 返回内存块的大小
     * @return size_t 
     */
    size_t getBaseSize() const {return baseSize_;}

    /**
     * @brief 返回可读数据大小
     * @return size_t 
     */
    size_t getReadSize() const {return size_ - position_;}

    /**
     * @brief 是否设置为小端
     * @return true 
     * @return false 
     */
    bool isLittleEndian() const;

    /**
     * @brief 设置是否为小端
     * @param val 
     */
    void setIsLittleEndian(bool val);

    /**
     * @brief 将ByteArray里面的数据[position_, size_)转成std::string
     * @return std::string 
     */
    std::string toString() const;

    /**
     * @brief 将ByteArray里面的数据[position_, size_)转成16进制的std::string(格式：FF FF FF) 
     * @return std::string 
     */
    std::string toHexString() const;

    /**
     * @brief 获取可读取的缓存，保存成iovec数组
     * @param buffers 保存可读取数据的iovec数组
     * @param len 读取数据的长度，如果len > getReadSize() 则 len = getReadSize()
     * @return uint64_t 返回实际数据的长度
     */
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;

    /**
     * @brief 获取可读取的缓存，保存成iovec数组
     * @param buffers 保存可读取数据的iovec数组
     * @param len 读取数据的长度，如果len > getReadSize() 则 len = getReadSize()
     * @param position 读取数据的位置
     * @return uint64_t 返回实际数据的长度
     */
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;

    /**
     * @brief 获取可写入的缓存，保存成iovec数组
     * @param buffers 保存可写入的内存的iovec数组
     * @param len 写入长度
     * @return uint64_t 返回实际的长度
     * @post 如果(position_ + len) > capacity_ 则 capacity_扩容N个节点以容纳len长度
     */
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

    /**
     * @brief 返回数据的长度
     * @return size_t 
     */
    size_t getSize() const {return size_;}

private:
    /**
     * @brief 扩容ByteArray，使其可以容纳size个数据(如果原本可以容纳，则不扩容) 
     * @param size 
     */
    void addCapacity(size_t size);

    /**
     * @brief 获取当前的可写入容量
     * @return size_t 
     */
    size_t getCapacity() const {return capacity_ - position_;}
private:
    // 内存块的大小 
    size_t baseSize_;
    // 当前操作位置
    size_t position_;
    // 当前的总容量
    size_t capacity_;
    // 当前数据的大小
    size_t size_;
    // 字节序，默认大端 网络字节序一般是指大端传输，人们常用数字读取方式也是大端。
    int8_t endian_;
    // 第一个内存块的指针
    Node* root_;
    // 当前操作的内存块指针
    Node* cur_;
};

}