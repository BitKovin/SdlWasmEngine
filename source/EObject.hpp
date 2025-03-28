#pragma once

class EObject
{
public:

	void Dispose()
	{

		OnDispose();

		delete this;
	}

protected:

	virtual void OnDispose()
	{

	}

};