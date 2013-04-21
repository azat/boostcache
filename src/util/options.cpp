
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include <fstream>
#include <iostream>
#include <limits.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "kernel/with_trace_exception.h"
#include "util/options.h"
#include "util/log.h"
#include "config.h"


namespace Util
{
    template <class Type>
    Type Options::getValue(const char *optionKey) const
    {
        // From expandedOptions
        ExpandedOptions::const_iterator it = expandedOptions.find(optionKey);
        if (it != expandedOptions.end()) {
            return boost::any_cast<Type>(it->second);
        }

        // From variablesMap
        if (IGNORE_NOT_EXISTED_ITEMS && !variablesMap.count(optionKey)) {
            return Type();
        }
        return variablesMap[optionKey].as<Type>();
    }

    bool Options::getValue(const char *optionKey) const
    {
        // From expandedOptions
        ExpandedOptions::const_iterator it = expandedOptions.find(optionKey);
        if (it != expandedOptions.end()) {
            return true;
        }

        // From variablesMap
        if (IGNORE_NOT_EXISTED_ITEMS && !variablesMap.count(optionKey)) {
            return false;
        }
        return true;
    }

    void Options::tryParseOptions(int argc, char **argv)
    {
        try {
            parseOptions(argc, argv);

            // TODO: not sure is it normal to check this here.
            if (variablesMap.count("help")) {
                std::cout << (*this);
                exit(EXIT_SUCCESS);
            }

            if (variablesMap.count("version")) {
                std::cout
                    << BOOSTCACHE_NAME ":" << std::endl
                    << '\t' << "Git version: " << BOOSTCACHE_VERSION_GIT << std::endl
                    << '\t' << "Version:     " <<  BOOSTCACHE_VERSION_FLOAT << std::endl
                    << std::endl
                    << "See --help for more information" << std::endl;
                exit(EXIT_SUCCESS);
            }

            /**
             * Install log level here for now,
             * maybe we need to move this to childs
             */
            Util::installLoggerLevel(getValue<int>("logLevel"));

            std::string fileFormat = getValue<std::string>("logFile");
            if (fileFormat.size()) {
                Util::installLoggerFile(fileFormat);
            }
        } catch(const std::exception& exception) {
            if (PRINT_HELP_ON_ERROR) {
                std::cerr << (*this);
            }

            std::cerr << exception.what() << std::endl;
            abort();
        }
    }

    void Options::parseOptions(int argc, char **argv)
    {
        /* support for -vv -vvvv etc. */
        for (std::string s = "vv"; s.length() <= MAX_VERBOSE_LEVEL; s.append("v")) {
            hiddenOptions.add_options()(s.c_str(), "verbose");
        }

        visibleOptions.add_options()
            ("help,h", "Produce help message")
            ("verbose,v",
             "Enable verbosity (optionally specify level, more v - more debug messages)")
            ("version,V", "Print version")
            ("config,c", boost::program_options::value<std::string>(),
             "Setup custom config file")
            ("logFile,l", boost::program_options::value<std::string>(),
             "Output log (used instead of stdout, can contain modifiers)")
        ;

        additionalOptions();

        /* don't allow guessing - creates ambiguities when some options are
         * prefixes of others. allow long disguises and don't allow guessing
         * to get away with our vvvvvvv trick. */
        int mongoStyle = (
            (
                (boost::program_options::command_line_style::unix_style ^ boost::program_options::command_line_style::allow_guessing)
                | boost::program_options::command_line_style::allow_long_disguise
            )
            ^ boost::program_options::command_line_style::allow_sticky);

        OptionsDescription allOptions;
        allOptions.add(hiddenOptions);
        allOptions.add(visibleOptions);
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).style(mongoStyle).options(allOptions).run(), variablesMap);

        std::string config = getValue<std::string>("config");
        if (config.size()) {
            std::ifstream fileStream(config.c_str());
            if (!fileStream.is_open()) {
                throw Exception("Could not read from config file");
            }

            std::stringstream stringStream;
            parseConfigFile(fileStream, stringStream);
            boost::program_options::store(boost::program_options::parse_config_file(stringStream, allOptions), variablesMap);
            fileStream.close();
        }

        boost::program_options::notify(variablesMap);

        expandedOptions["logLevel"] = int(variablesMap.count("verbose") ? 1 : 0);
        for (std::string s = "vv"; s.length() <= MAX_VERBOSE_LEVEL; s.append("v")) {
            if (variablesMap.count(s)) {
                expandedOptions["logLevel"] = int(s.length());
            }
        }
    }

    void Options::parseConfigFile(std::istream &fileStream, std::stringstream &stringStream)
    {
        std::string tmpConfigString;

        char line[PATH_MAX];

        while (fileStream) {
            fileStream.getline(line, PATH_MAX);
            tmpConfigString = line;

            // TODO: normal parsing of phrases in quotes.
            boost::algorithm::replace_first(tmpConfigString, "'", "");
            boost::algorithm::replace_first(tmpConfigString, "\"", "");
            boost::algorithm::trim_right_if(tmpConfigString, boost::algorithm::is_any_of("'\""));

            stringStream << tmpConfigString << std::endl;
        }
    }

    #define TEST_AND_PRINT(typeValue, name, type, output) \
        if (typeValue == typeid(type)) {                  \
            output << getValue<type>(name.c_str());       \
            return;                                       \
        }
    void Options::dumpValue(const std::type_info &type, const std::string &name, std::ostream &output)
    {
        output << name << "=";
        TEST_AND_PRINT(type, name, std::string, output);
        TEST_AND_PRINT(type, name, int, output);
        TEST_AND_PRINT(type, name, float, output);
        TEST_AND_PRINT(type, name, bool, output);

        throw WithTraceException("Unknown type");
    }

    void Options::dumpOptions()
    {
        for (VariablesMap::const_iterator iter = variablesMap.begin(); iter != variablesMap.end(); ++iter) {
            std::string name = (*iter).first;
            const std::type_info &type = iter->second.value().type();

            dumpValue(type, name, std::cerr);
        }
    }
}
