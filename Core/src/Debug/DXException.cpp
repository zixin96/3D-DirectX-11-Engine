#include "DXException.h"

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
	// oss.str() will be destroyed at the end of this function, 
	// thus, we need a buffer to store that information
	whatBuffer_ = oss.str();
	return whatBuffer_.c_str();
}

const char* DXException::GetType() const noexcept
{
	return "3D Game Programming DirectXEngine Exception";
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
