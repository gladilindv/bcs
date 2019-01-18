#pragma once
class CWinSvc
{
public:
	CWinSvc() = default;
	~CWinSvc() = default;
	
	void Install();
	void Remove();
	void Dispatch();
};
