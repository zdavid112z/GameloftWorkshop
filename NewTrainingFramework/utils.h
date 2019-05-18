#pragma once

#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <string>
#include <cassert>

#define ASSERT(expr, msgFormat, ...) { if(!(expr)) { printf("Assertion failed in %s on line %d!\n%s evaluates to false\n", __FILE__, __LINE__, #expr); printf(msgFormat, __VA_ARGS__); printf("\n"); assert(0); } }
#define CheckGLError() { GLenum err = glGetError(); if(err) printf("GL Error: %d\nFile: %s\nLine: %d\n\n", err, __FILE__, __LINE__); }

class Utils
{
public:
	constexpr static uint16_t SimpleHash16(const uint8_t* key, size_t len, uint16_t mult = 31, uint16_t mod = 65519);
	constexpr static uint16_t SimpleHash16(const char* str, uint16_t mult = 31, uint16_t mod = 65519);
	constexpr static uint16_t SimpleHash16(const std::string& str, uint16_t mult = 31, uint16_t mod = 65519);

	constexpr static uint32_t SimpleHash32(const uint8_t* key, size_t len, uint32_t mult = 31, uint32_t mod = 1000000007);
	constexpr static uint32_t SimpleHash32(const char* str, uint32_t mult = 31, uint32_t mod = 1000000007);
	constexpr static uint32_t SimpleHash32(const std::string& str, uint32_t mult = 31, uint32_t mod = 1000000007);

	constexpr static uint32_t Murmur3_32(const uint8_t* key, size_t len, uint32_t seed = 0);
	constexpr static uint32_t Murmur3_32(const char* str, uint32_t seed = 0);
	constexpr static uint32_t Murmur3_32(const std::string& str, uint32_t seed = 0);

	constexpr static uint32_t Hash(const uint8_t* key, size_t len);
	constexpr static uint32_t Hash(const char* str);
	constexpr static uint32_t Hash(const std::string& str);
	
	constexpr static uint32_t murmur3Seed = 0;

	static std::string Trim(const std::string& str);
	static std::string ReadFile(const std::string& path);
	static std::string AddLineNumbers(const std::string& str);
	static bool StartsWith(const std::string& str, const std::string& prefix);
	static bool EndsWith(const std::string& str, const std::string& suffix);
};

constexpr uint32_t Utils::Hash(const uint8_t* key, size_t len)
{
	return SimpleHash32(key, len);
}

constexpr uint32_t Utils::Hash(const char* str)
{
	return SimpleHash32(str);
}

constexpr uint32_t Utils::Hash(const std::string& str)
{
	return SimpleHash32(str);
}

constexpr uint32_t Utils::Murmur3_32(const std::string& str, uint32_t seed)
{
	return Murmur3_32((const uint8_t*)str.c_str(), str.size(), seed);
}

constexpr uint32_t Utils::Murmur3_32(const char* str, uint32_t seed)
{
	return Murmur3_32((const uint8_t*)str, strlen(str), seed);
}

constexpr uint32_t Utils::Murmur3_32(const uint8_t* key, size_t len, uint32_t seed)
{
	uint32_t h = seed;
	if (len > 3) {
		const uint32_t* key_x4 = (const uint32_t*)key;
		size_t i = len >> 2;
		do {
			uint32_t k = *key_x4++;
			k *= 0xcc9e2d51;
			k = (k << 15) | (k >> 17);
			k *= 0x1b873593;
			h ^= k;
			h = (h << 13) | (h >> 19);
			h = (h * 5) + 0xe6546b64;
		} while (--i);
		key = (const uint8_t*)key_x4;
	}
	if (len & 3) {
		size_t i = len & 3;
		uint32_t k = 0;
		key = &key[i - 1];
		do {
			k <<= 8;
			k |= *key--;
		} while (--i);
		k *= 0xcc9e2d51;
		k = (k << 15) | (k >> 17);
		k *= 0x1b873593;
		h ^= k;
	}
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

constexpr uint16_t Utils::SimpleHash16(const uint8_t* key, size_t len, uint16_t mult, uint16_t mod)
{
	uint32_t h = 0;
	for (; len--; key++)
		h = (h * mult + *key) % mod;
	return h;
}

constexpr uint16_t Utils::SimpleHash16(const char* str, uint16_t mult, uint16_t mod)
{
	uint32_t h = 0;
	for (; *str != 0; str++)
		h = (h * mult + *str) % mod;
	return h;
}

constexpr uint16_t Utils::SimpleHash16(const std::string& str, uint16_t mult, uint16_t mod)
{
	return SimpleHash16(str.c_str(), mult, mod);
}

constexpr uint32_t Utils::SimpleHash32(const uint8_t* key, size_t len, uint32_t mult, uint32_t mod)
{
	uint32_t h = 0;
	for (; len--; key++)
		h = (uint64_t(h) * mult + *key) % mod;
	return h;
}

constexpr uint32_t Utils::SimpleHash32(const char* str, uint32_t mult, uint32_t mod)
{
	uint32_t h = 0;
	for (; *str != 0; str++)
		h = (uint64_t(h) * mult + *str) % mod;
	return h;
}

constexpr uint32_t Utils::SimpleHash32(const std::string& str, uint32_t mult, uint32_t mod)
{
	return SimpleHash32(str.c_str(), mult, mod);
}

template <uint32 hash>
static inline constexpr uint32 _force_compile_hash()
{
	return hash;
}

#define HASH(string) (_force_compile_hash< Utils::Hash(string) >())
