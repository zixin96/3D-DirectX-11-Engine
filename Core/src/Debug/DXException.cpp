#include "DXException.h"
#include <sstream>


DXException::DXException(int line, const char* file) noexcept
	:
	line_(line),
	file_(file)
{
}

const char* DXException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
	whatBuffer_ = oss.str();
	return whatBuffer_.c_str();
}

const char* DXException::GetType() const noexcept
{
	return "DirectXEngine Exception";
}

int DXException::GetLine() const noexcept
{
	return line_;
}

const std::string& DXException::GetFile() const noexcept
{
	return file_;
}

std::string DXException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file_ << std::endl
		<< "[Line] " << line_;
	return oss.str();
}
