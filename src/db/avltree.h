
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#pragma once

#include "db/interface.h"

#include <boost/intrusive/avl_set_hook.hpp>
#include <boost/intrusive/avltree.hpp>
#include <boost/utility.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <memory>
#include <utility>
#include <list>
#include <string>


/**
 * @brief Avl tree using boost::intrusive
 *
 * Thread-safe (TODO: improve thread-safe support)
 */

namespace Db
{
    class AvlTree : public Interface
    {
    public:
        AvlTree();

        std::string get(const Command::Arguments& arguments);
        std::string set(const Command::Arguments& arguments);

    private:
        static std::hash<Key> m_keyHashFunction;

        /**
         * TODO: move to private class or just from header away
         */
        class Node
            : public boost::intrusive::avl_set_member_hook< boost::intrusive::optimize_size<true> >
        {
        public:
            struct Data
            {
                size_t internalKey;
                Value value;

                Data(Key key, Value value = "")
                    : internalKey(m_keyHashFunction(key))
                    , value(value)
                {}
            };

            boost::intrusive::avl_set_member_hook<> member_hook;

            // TODO: std::move()
            Node(Data data) : m_data(data) {}

            const Data &get() const
            {
                return m_data;
            }

            friend bool operator <(const Node& left, const Node& right)
            {
                return (left.m_data.internalKey < right.m_data.internalKey);
            }

            friend bool operator ==(const Node& left, const Node& right)
            {
                return (left.m_data.internalKey == right.m_data.internalKey);
            }

        private:
            Data m_data;
        };

        typedef boost::intrusive::member_hook< Node, boost::intrusive::avl_set_member_hook<>, &Node::member_hook > MemberHook;
        typedef boost::intrusive::avltree< Node, MemberHook > Tree;
        std::unique_ptr<Tree> m_tree;

        typedef std::list<Node> Nodes;
        Nodes m_nodes;

        /**
         * TODO: maybe move to ThreadSafe wrapper
         */
        boost::shared_mutex m_access;
    };
}
