#ifndef TOCONFIGURATION_H
#define TOCONFIGURATION_H

#ifdef TOAD
#define CONFIG_FILE "/.toadrc"
#define DEF_CONFIG_FILE "/etc/toadrc"
#else
#define CONFIG_FILE "/.torarc"
#define DEF_CONFIG_FILE "/etc/torarc"
#endif

#include <map>

#include <loki/Singleton.h>
#include <qstring.h>

#include <qapplication.h>
//

// A little magic to get lrefresh to work and get a check on qApp

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

class toConfiguration
{
public :
    toConfiguration();
    virtual ~toConfiguration();

    /**
     * Load configuration from file.
     */
    void loadConfig(void);

    /**
     * Save configuration to file.
     */
    void saveConfig(void);

   /**
     * Get value of a setting.
     *
     * Setting names are hierachical separated by ':' instead of '/' usually used
     * in filenames. As an example all settings for the tool 'Example' would be
     * under the 'Example:{settingname}' name.
     *
     * @param tag The name of the configuration setting.
     * @param def Default value of the setting, if it is not available.
     */
    const QString &globalConfig(const QCString &tag, const QCString &def);

    /**
     * Load a string to string map from file saved by the @ref saveMap function.
     * @param filename Filename to load
     * @param map Reference to the map to fill with the new values.
     */
    void loadMap(const QString &filename, std::map<QCString, QString> &map);

    /**
         * A map containing the available configuration settings. By convention the
         * character ':' is used to separate parts of the path.
         *
         * @see globalConfig
         * @see globalSetConfig
         * @see config
         * @see setConfig
         */
    bool saveMap(const QString &file, std::map<QCString, QString> &pairs);

    /**
     * Change a setting. Depending on the implementation this can change the
     * contents on disk or not.
     *
     * Setting names are hierachical separated by ':' instead of '/' usually used
     * in filenames. As an example all settings for the tool 'Example' would be
     * under the 'Example:{settingname}' name.
     *
     * @param tag The name of the configuration setting.
     * @param def Contents of this setting.
     */
    void globalSetConfig(const QCString &tag, const QString &value);
    /**
     * Remove a setting. Can be usefull for removing sensetive information.
     * @param tag The name of the configuration setting to remove.
     */
    void globalEraseConfig(const QCString &tag);

    /**
     * Get tool specific settings.
     *
     * Setting names are hierachical separated by ':' instead of '/' usually used
     * in filenames. As an example all settings for the tool 'Example' would be
     * under the 'Example:{settingname}' name.
     *
     * @param tag The name of the configuration setting.
     * @param def Contents of this setting.
     */
    const QString &config(const QCString &tag, const QCString &def, const QCString &name);
    /**
     * Change toolspecific setting. Depending on the implementation this can change the
     * contents on disk or not.
     *
     * Setting names are hierachical separated by ':' instead of '/' usually used
     * in filenames. As an example all settings for the tool 'Example' would be
     * under the 'Example:{settingname}' name.
     *
     * @param tag The name of the configuration setting.
     * @param def Default value of the setting, if it is not available.
     */
    void setConfig(const QCString &tag, const QString &value, const QCString name = "");
    /**
     * Remove a toolspecific setting. Can be usefull for removing sensetive information.
     * @param tag The name of the configuration setting to remove.
     */
    void eraseConfig(const QCString &tag, const QCString& name);

private :
    std::map<QCString, QString> Configuration;
};

typedef Loki::SingletonHolder<toConfiguration> toConfigurationSingle;

#endif
