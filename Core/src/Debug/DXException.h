#pragma once
#include <exception>
#include <string>

/**
 * \brief The parent class of all our exceptions
 *
 * It's a good practice to inherit from std::exception
 */
class DXException : public std::exception
{
public:
	DXException(int line, const char* file) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;
private:
	// Exception will capture the following data:
	// the line number and file name of where the exception is thrown
	int line_;
	std::string file_;
protected:
	// a buffer used to store exception information
	mutable std::string whatBuffer_;
};
