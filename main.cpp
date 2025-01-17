#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <vector>
#include <charconv>
#include <sstream>

#define FMT_HEADER_ONLY
#include "fmt/format.h"

#define HAS_FORMAT (__cplusplus >= 202004L)
#if HAS_FORMAT
#include <format>
#endif

#define C4CORE_SINGLE_HDR_DEFINE_NOW
#include "c4core.hpp"


#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

using namespace std;

static int g_Global;
static const int kMaxThreads = 10;
static const int kIterations = 2000000;
static const size_t kExpect = 114888893;

static void do_snprintf(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		snprintf(buf, 100, "%i", i + g_Global);
		sum += strlen(buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of snprintf %i!\n", kExpect, sum, index);
		exit(1);
	}
}

#ifndef __linux__
static void do_snprintf_l(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		#if defined(_MSC_VER)
		_snprintf_l(buf, 100, "%i", NULL, i + g_Global);
		#else
		snprintf_l(buf, 100, NULL, "%i", i + g_Global);
		#endif
		sum += strlen(buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of snprintf_l %i!\n", kExpect, sum, index);
		exit(1);
	}
}
#endif

static void do_stb_snprintf(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		stbsp_snprintf(buf, 100, "%i", i + g_Global);
		sum += strlen(buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of stbsp_snprintf %i!\n", kExpect, sum, index);
		exit(1);
	}
}

static void do_stringstream(int index)
{
	size_t sum = 0;
	stringstream buf;
	//locale loc("C");
	//buf.imbue(loc);
	for (int i = 0; i < kIterations; ++i) {
		buf.seekp(0);
		buf << (i + g_Global);
		int len = (int)buf.tellp();
		buf.seekg(0);
		int firstchar = buf.get();
		sum += len + firstchar;
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of stringstream %i!\n", kExpect, sum, index);
		exit(1);
	}
}

static void do_fmt(int index)
{
	size_t sum = 0;
	fmt::memory_buffer buf;
	for (int i = 0; i < kIterations; ++i) {
		buf.clear();
		fmt::format_to(fmt::appender(buf), "{}", i + g_Global);
		sum += buf.size() + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of fmt %i!\n", kExpect, sum, index);
		exit(1);
	}
}

#if HAS_FORMAT
static void do_format_to(int index)
{
	size_t sum = 0;
	char buf[100];
	//locale loc("C");
	for (int i = 0; i < kIterations; ++i) {
		const auto res = format_to_n(buf, 100, /*loc,*/ "{}", i + g_Global);
		sum += (res.out - buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of format_to_n %i!\n", kExpect, sum, index);
		exit(1);
	}
}
#endif

#ifdef _MSC_VER
static void do_itoa(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		_itoa(i + g_Global, buf, 10);
		sum += strlen(buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread %i of itoa!\n", kExpect, sum, index);
		exit(1);
	}	
}
#endif

static void do_to_chars(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		to_chars_result res = to_chars(buf, buf+100, i + g_Global);
		sum += (res.ptr - buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread %i of to_chars!\n", kExpect, sum, index);
		exit(1);
	}
}

static void do_c4_write_dec(int index)
{
	size_t sum = 0;
	char buf_[100];
    c4::substr buf = buf_;
	for (int i = 0; i < kIterations; ++i) {
		size_t len = c4::write_dec(buf, i + g_Global);
		sum += len + buf[0];
	}
	if (sum != kExpect)
    {
		printf("    sum was supposed to be %zi, but got %zi in thread %i of c4::write_dec!\n", kExpect, sum, index);
		exit(1);
	}
}

static void do_c4_itoa(int index)
{
	size_t sum = 0;
	char buf_[100];
    c4::substr buf = buf_;
	for (int i = 0; i < kIterations; ++i) {
		size_t len = c4::itoa(buf, i + g_Global);
		sum += len + buf[0];
	}
	if (sum != kExpect)
    {
		printf("    sum was supposed to be %zi, but got %zi in thread %i of c4::itoa!\n", kExpect, sum, index);
		exit(1);
	}
}

static void do_c4_to_chars(int index)
{
	size_t sum = 0;
	char buf_[100];
    c4::substr buf = buf_;
	for (int i = 0; i < kIterations; ++i) {
		size_t len = c4::to_chars(buf, i + g_Global);
		sum += len + buf[0];
	}
	if (sum != kExpect)
    {
		printf("    sum was supposed to be %zi, but got %zi in thread %i of c4::to_chars!\n", kExpect, sum, index);
		exit(1);
	}
}

typedef void (*thread_func)(int index);

static void do_test_with_func(const char* name, thread_func func)
{
	printf("==== Test %s:\n", name);
	vector<float> times;
	for (int i = 1; i <= kMaxThreads; ++i) {
		printf("%i threads... ", i);
		vector<thread> threads;
		auto t0 = chrono::steady_clock::now();
		for (int j = 0; j < i; ++j)
			threads.emplace_back(thread(func, j));
		for (auto &t : threads)
			t.join();
		auto t1 = chrono::steady_clock::now();
		chrono::duration<float, milli> ms = t1 - t0;
		float msf = ms.count();
		printf("  %.1fms\n", msf);
		times.push_back(msf);
	}
	for (size_t i = 0; i < times.size(); ++i)
	{
		printf("%.1f\n", times[i]);
	}
}

int main(int argc, const char**)
{
	g_Global = argc;
	do_test_with_func("snprintf", do_snprintf);
#ifndef __linux__
	do_test_with_func("snprintf_l", do_snprintf_l);
#endif
	do_test_with_func("stb_snprintf", do_stb_snprintf);
	do_test_with_func("stringstream", do_stringstream);
	do_test_with_func("fmt", do_fmt);
#if HAS_FORMAT
	do_test_with_func("format_to_n", do_format_to);
#endif
#ifdef _MSC_VER
	do_test_with_func("itoa", do_itoa);
#endif
	do_test_with_func("to_chars", do_to_chars);
	do_test_with_func("c4_write_dec", do_c4_write_dec);
	do_test_with_func("c4_itoa", do_c4_itoa);
	do_test_with_func("c4_to_chars", do_c4_to_chars);
	return 0;
}
