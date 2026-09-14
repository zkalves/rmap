# LanguageManager

### 1. Class Overview

`LanguageManager` is a singleton manager responsible for dynamic internationalization (i18n) and runtime locale switching in **rmap**. It manages translation catalogues encoded in JSON or compiled Qt QM formats (`:/translations/rmap_<lang>.json`), translates UI elements via `JsonTranslator`, and notifies widgets of language switches via the `languageChanged` signal.

### 2. Project Structure and Dependencies

Defined in `src/LanguageManager.hpp` and implemented in `src/LanguageManager.cpp`.
- Initialized during application startup in `main.cpp`.
- Utilized by `RegMapWindow`, `PreferencesWindow`, and custom widgets to support dynamic UI re-translation.
- Supports built-in translation catalogs for English (`en`), Spanish (`es`), German (`de`), French (`fr`), Simplified Chinese (`zh_CN`), Japanese (`ja`), and Portuguese (`pt_BR`).

Build Requirements:
- Qt 6 modules: `QtCore`, `QtGui`, `QtWidgets`
- Embedded JSON translations registered in Qt resource system

### 3. Class Hierarchy and Role

`LanguageManager` inherits from `QObject`:
- Singleton design pattern accessed via `LanguageManager::instance()`.
- Dispatches `languageChanged(QString code)` signal when active locale changes.
- Installs and removes custom `JsonTranslator` instances into `QCoreApplication`.

`JsonTranslator` inherits from `QTranslator`:
- Parses lightweight key-value or contextual translation dictionaries in JSON format.
- Implements `translate(context, sourceText, disambiguation, n)` for zero-overhead translation lookup.

### 4. Struct LanguageInfo

```cpp
struct LanguageInfo {
    QString code;         // Language code: "en", "es", "de", "fr", "zh_CN", "ja", "pt_BR"
    QString name;         // English display name: "English", "Spanish", "German"
    QString nativeName;   // Native display name: "English", "Español", "Deutsch"
    QString resourcePath; // Resource path: ":/translations/rmap_es.json"
    bool isBuiltIn;       // Indicates if bundled in application resources
    QString displayName() const; // Formats "Native Name (English Name)"
};
```

### 5. Public Methods

#### static LanguageManager& instance()
Returns the process-wide singleton instance.

#### const QList<LanguageInfo>& availableLanguages() const
Returns the list of all registered languages.

#### QStringList languageCodes() const noexcept
Returns a string list of available ISO language codes.

#### QStringList languageNames() const noexcept
Returns a string list of English language names.

#### QString currentLanguage() const
Returns the currently active language code (e.g., `"en"`).

#### QString currentLanguageName() const
Returns the human-readable display name of the current language.

#### LanguageInfo currentLanguageInfo() const
Returns the full metadata descriptor for the active language.

#### bool setLanguage(const QString &codeOrName)
Switches the active language to the specified code or name. Loads the corresponding translator, installs it in `QCoreApplication`, and emits `languageChanged()`.

#### bool hasLanguage(const QString &codeOrName) const
Returns `true` if a language matching the code or name is registered.

#### LanguageInfo languageInfo(const QString &codeOrName) const
Returns the language metadata matching the given code or name.

### 6. Signals

#### void languageChanged(const QString &languageCode)
Emitted whenever the application language is successfully updated. UI windows connect to this signal to call `retranslateUi()`.

### 7. Thread Safety

`LanguageManager` is designed for use on the **main GUI thread**.

### 8. Usage Example

```cpp
#include "LanguageManager.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Switch to Spanish
    LanguageManager::instance().setLanguage("es");

    // Query active language
    qDebug() << "Active language:" << LanguageManager::instance().currentLanguage();

    return app.exec();
}
```
