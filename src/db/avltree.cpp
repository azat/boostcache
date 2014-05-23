
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "avltree.h"
#include "server/jsvm.h"
#include "kernel/exception.h"
#include "util/log.h"

namespace Db
{
    std::hash<AvlTree::Key> AvlTree::m_keyHashFunction = std::hash<AvlTree::Key>();

    AvlTree::AvlTree()
        : Interface()
        , m_deleteDisposer(m_nodes)
        , m_tree(new Tree)
    {
    }

    std::string AvlTree::get(const CommandHandler::Arguments &arguments)
    {
        Node findMe(arguments[1]);

        // get shared lock
        boost::shared_lock<boost::shared_mutex> lock(m_access);

        Tree::const_iterator found = m_tree->find(findMe);
        if (found == m_tree->end()) {
            return CommandHandler::REPLY_NIL;
        }
        return CommandHandler::toReplyString(found->get().value);
    }

    std::string AvlTree::set(const CommandHandler::Arguments &arguments)
    {
        // get exclusive lock
        boost::upgrade_lock<boost::shared_mutex> lock(m_access);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        Node findMe(arguments[1]);
        Tree::iterator found = m_tree->find(findMe);
        if (found != m_tree->end()) {
            found->get().value = arguments[2] /* value */;
            return CommandHandler::REPLY_OK;
        }

        m_nodes.push_back(Node(Node::Data(arguments[1] /* key */,
                                          arguments[2] /* value */)));
        m_nodes.back().get().listIterator = --m_nodes.end();
        m_tree->insert_unique(m_nodes.back());

        return CommandHandler::REPLY_OK;
    }

    std::string AvlTree::del(const CommandHandler::Arguments &arguments)
    {
        // get exclusive lock
        boost::upgrade_lock<boost::shared_mutex> lock(m_access);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        Node findMe(arguments[1]);
        Tree::const_iterator found = m_tree->find(findMe);
        if (found == m_tree->end()) {
            return CommandHandler::REPLY_FALSE;
        }
        m_tree->erase_and_dispose(found, m_deleteDisposer);

        return CommandHandler::REPLY_TRUE;
    }

    std::string AvlTree::foreach(const CommandHandler::Arguments &arguments)
    {
        JsVm vm(arguments[1]);
        if (!vm.init()) {
            return CommandHandler::REPLY_ERROR;
        }

        /**
         * Get shared lock
         *
         * But when we will allow user to change values, we need to convert it
         * to exclusive (and personally I think that this is must-have feature).
         */
        boost::shared_lock<boost::shared_mutex> lock(m_access);

        for (Node &node : m_nodes) {
            std::string &key = node.get().key;
            std::string &value = node.get().value;

            try {
                value = vm.call(key, value);
            } catch (const Exception &e) {
                LOG(error) << e.getMessage();
                LOG(error) << "Will not continue";
                return CommandHandler::REPLY_ERROR;
            }
        }

        return CommandHandler::REPLY_TRUE;
    }
}
