
/**
 * This file is part of the boostcache package.
 *
 * TODO: add default configs paths
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

#include <map>
#include <ostream>
#include <istream>
#include <sstream>
#include <string>


namespace Util
{
    class Options : boost::noncopyable
    {
    public:
        typedef boost::program_options::variables_map VariablesMap;
        typedef boost::program_options::options_description OptionsDescription;
        typedef std::map<std::string, boost::any> ExpandedOptions;

        Options() {}
        void parse(int argc, char **argv)
        {
            tryParseOptions(argc, argv);
        }

        template <class Type>
        Type getValue(const char *optionKey) const;
        bool getValue(const char *optionKey) const;

        void help(std::ostream& out)
        {
            out << (*this);
        }

        friend std::ostream& operator <<(std::ostream &out, Options &options)
        {
            out << options.visibleOptions;
            return out;
        }

        void dumpOptions();

    protected:
        OptionsDescription visibleOptions;
        virtual void additionalOptions() = 0;

    private:
        static const bool IGNORE_NOT_EXISTED_ITEMS = true;
        static const bool PRINT_HELP_ON_ERROR = false;
        static const size_t MAX_VERBOSE_LEVEL = 20;

        VariablesMap variablesMap;

        OptionsDescription hiddenOptions;

        ExpandedOptions expandedOptions;

        // Because of old gcc don't allow to call once contruction from another for the same class.
        // And in future we will add another construtor with default config files to parse in.
        void tryParseOptions(int argc, char **argv);
        void parseOptions(int argc, char **argv);
        void dumpValue(const std::type_info &type, const std::string &name, std::ostream &output);
        static void parseConfigFile(std::istream &fileStream, std::stringstream &stringStream);
    };
}
