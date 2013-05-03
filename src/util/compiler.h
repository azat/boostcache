
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

/**
 * Some compiler attributes and other things
 */

#pragma once

#define UNUSED(x) UNUSED_ ## x __attribute__((unused))
