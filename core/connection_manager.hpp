#ifndef _CONNECTION_MANAGER_HPP_
#define _CONNECTION_MANAGER_HPP_

#include <set>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

#include "./connection.hpp"

class ConnectionManager
    : public boost::noncopyable
{
    typedef std::set< IConnectionPtr > ConnectionsPtr;

public:

    template< typename Function, class... Args >
    void forEach(
        Function && func,
        Args && ...args
    );

    template< typename Predicate, typename Function, class... Args >
    void forEachIf(
        Predicate && predicate,
        Function && func,
        Args && ...args
    );

    void add(
        IConnectionPtr & connection
    );

    void remove(
        IConnectionPtr const & connection
    );

private:

    boost::mutex m_mutex;
    ConnectionsPtr m_connections;
};

template<
    typename Function,
    class... Args
>
void ConnectionManager::forEach(
    Function && func,
    Args && ...args
)
{
    boost::lock_guard< boost::mutex > lock( m_mutex );

    for( auto & connection : m_connections )
    {
        func( connection, std::forward< Args >( args )... );        
    }
}

template<
    typename Predicate,
    typename Function,
    class... Args
>
void ConnectionManager::forEachIf(
    Predicate && predicate,
    Function && func,
    Args && ...args
)
{
    boost::lock_guard< boost::mutex > lock( m_mutex );

    for( auto & connection : m_connections )
    {
        if( predicate( connection ) )
        {
            func( connection, std::forward< Args >( args )... );        
        }
    }
}

void ConnectionManager::add(
    IConnectionPtr & connection
)
{
    boost::lock_guard< boost::mutex > lock( m_mutex );

    m_connections.insert( connection ); 
}

void ConnectionManager::remove(
    IConnectionPtr const & connection
)
{
    boost::lock_guard< boost::mutex > lock( m_mutex );

    auto const pos = m_connections.find( connection );

    if( pos != m_connections.end() ){
        m_connections.erase( pos );
    }
}

#endif

