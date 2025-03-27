#ifndef EOBJECT_HPP
#define EOBJECT_HPP

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

#endif