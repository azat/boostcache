
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
        , m_tree(new Tree)
    {
    }

    std::string AvlTree::get(const Command::Arguments& arguments)
    {
        Node findMe(arguments[1]);
        Tree::const_iterator found = m_tree->find(findMe);
        if (found == m_tree->end()) {
            return Command::REPLY_NIL;
        }
        return valueToReplyString(found->get().value);
    }

    std::string AvlTree::set(const Command::Arguments& arguments)
    {
        // TODO: avoid this check in every command (see Commands notes)
        if (arguments.size() != 3) {
            return Command::REPLY_ERROR;
        }

        m_nodes.push_back(Node(Node::Data(arguments[1] /* key */,
                                          arguments[2] /* value */)));
        m_tree->insert_unique(m_nodes.back());

        return Command::REPLY_OK;
    }
}