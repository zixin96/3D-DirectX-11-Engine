#pragma once

/**
 * \brief DxgiInfoManager class retrieves information coming out of D3D debug layer so that we can pass them to our debug window 
 */
class DxgiInfoManager
{
public:
	DxgiInfoManager();
	~DxgiInfoManager() = default;

	DxgiInfoManager(const DxgiInfoManager&) = delete;
	DxgiInfoManager& operator=(const DxgiInfoManager&) = delete;

	void Set() noexcept;
	std::vector<std::string> GetMessages() const;
private:
	unsigned long long next_ = 0u;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> pDxgiInfoQueue_;
};
