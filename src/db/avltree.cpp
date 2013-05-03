
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "avltree.h"

namespace Db
{
    std::hash<AvlTree::Key> AvlTree::m_keyHashFunction = std::hash<AvlTree::Key>();

    AvlTree::AvlTree()
        : Interface()
        , m_deleteDisposer(m_nodes)
        , m_tree(new Tree)
    {
    }

    std::string AvlTree::get(const Command::Arguments& arguments)
    {
        Node findMe(arguments[1]);

        // get shared lock
        boost::shared_lock<boost::shared_mutex> lock(m_access);

        Tree::const_iterator found = m_tree->find(findMe);
        if (found == m_tree->end()) {
            return Command::REPLY_NIL;
        }
        return valueToReplyString(found->get().value);
    }

    std::string AvlTree::set(const Command::Arguments& arguments)
    {
        // get exclusive lock
        boost::upgrade_lock<boost::shared_mutex> lock(m_access);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        Node findMe(arguments[1]);
        Tree::iterator found = m_tree->find(findMe);
        if (found != m_tree->end()) {
            found->get().value = arguments[2] /* value */;
            return Command::REPLY_OK;
        }

        m_nodes.push_back(Node(Node::Data(arguments[1] /* key */,
                                          arguments[2] /* value */)));
        m_nodes.back().get().listIterator = --m_nodes.end();
        m_tree->insert_unique(m_nodes.back());

        return Command::REPLY_OK;
    }

    std::string AvlTree::del(const Command::Arguments& arguments)
    {
        // get exclusive lock
        boost::upgrade_lock<boost::shared_mutex> lock(m_access);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        Node findMe(arguments[1]);
        Tree::const_iterator found = m_tree->find(findMe);
        if (found == m_tree->end()) {
            return Command::REPLY_FALSE;
        }
        m_tree->erase_and_dispose(found, m_deleteDisposer);

        return Command::REPLY_OK;
    }
}
