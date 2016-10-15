#ifndef _SINGLETON_H_
#define _SINGLETON_H_

template<class T>
class Singleton {
protected:
	Singleton() {}
	Singleton(Singleton &);
	Singleton& operator=(const Singleton&);
public:
	static T & get_inst() {
		static T inst;
		return inst;
	}

};

#endif
