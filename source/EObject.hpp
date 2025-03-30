#pragma once

class EObject
{
public:

	void Dispose()
	{

		OnDispose();


	}

protected:

	virtual void OnDispose()
	{

	}

};