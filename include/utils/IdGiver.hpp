#ifndef IDGIVER_HPP
#define IDGIVER_HPP

/*!
 * \brief the user of this class must overload the operator++ and the operator= if he wants to
 * see his type working as id.
 */
template<class T>
class IdGiver
{
public:
	IdGiver(T initializer)
	{
		id = initializer;
	}

	T giveAutoId()
	{
		return (id++);
	}

	T giveActualId()
	{
		return (id);
	}
private:
	T id;
};

#endif // !_IDGIVER_HPP_
