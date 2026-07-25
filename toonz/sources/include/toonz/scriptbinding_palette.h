#pragma once

#ifndef SCRIPTBINDING_PALETTE_H
#define SCRIPTBINDING_PALETTE_H

#include "toonz/scriptbinding.h"
#include "toonz/scriptbinding_files.h"
#include "tpalette.h"
#include "tstream.h"
#include "tconvert.h"
#include "tsystem.h"

namespace TScriptBinding {

class DVAPI Palette final : public Wrapper {
  Q_OBJECT

  TPalette *m_palette;

public:
  Palette();
  explicit Palette(TPalette *palette);
  ~Palette();

  WRAPPER_STD_METHODS(Palette)
  Q_INVOKABLE QScriptValue toString();

  // Palette file I/O (.tpl)
  Q_INVOKABLE QScriptValue save(const QScriptValue &pathArg) {
    TFilePath fp;
    QScriptValue err = checkFilePath(context(), pathArg, fp);
    if (err.isError()) return err;

    if (fp.getType() != "tpl") {
      return context()->throwError(
          tr("Palette files must use the .tpl extension: %1")
              .arg(pathArg.toString()));
    }

    try {
      TSystem::touchParentDir(fp);

      TOStream os(fp);
      if (!os) {
        return context()->throwError(
            tr("Could not open palette for writing: %1")
                .arg(pathArg.toString()));
      }

      std::map<std::string, std::string> attributes;
      attributes["name"] = ::to_string(m_palette->getGlobalName());
      os.openChild("palette", attributes);
      m_palette->saveData(os);
      os.closeChild();
      m_palette->setDirtyFlag(false);

      return context()->thisObject();
    } catch (...) {
      return context()->throwError(
          tr("Exception writing palette %1").arg(pathArg.toString()));
    }
  }

  Q_INVOKABLE QScriptValue load(const QScriptValue &pathArg) {
    TFilePath fp;
    QScriptValue err = checkFilePath(context(), pathArg, fp);
    if (err.isError()) return err;

    if (fp.getType() != "tpl") {
      return context()->throwError(
          tr("Palette files must use the .tpl extension: %1")
              .arg(pathArg.toString()));
    }

    if (!TSystem::doesExistFileOrLevel(fp)) {
      return context()->throwError(
          tr("Palette file does not exist: %1").arg(pathArg.toString()));
    }

    TPalette *loadedPalette = nullptr;
    try {
      TIStream is(fp);
      if (!is) {
        return context()->throwError(
            tr("Could not open palette for reading: %1")
                .arg(pathArg.toString()));
      }

      std::string tagName;
      if (!is.matchTag(tagName) || tagName != "palette") {
        return context()->throwError(
            tr("File is not a valid OpenToonz palette: %1")
                .arg(pathArg.toString()));
      }

      std::string globalName;
      is.getTagParam("name", globalName);

      loadedPalette = new TPalette();
      loadedPalette->loadData(is);
      loadedPalette->setGlobalName(::to_wstring(globalName));
      is.matchEndTag();
      loadedPalette->setPaletteName(fp.getWideName());
      loadedPalette->setDirtyFlag(false);

      loadedPalette->addRef();
      if (m_palette) m_palette->release();
      m_palette = loadedPalette;

      return context()->thisObject();
    } catch (...) {
      if (loadedPalette) delete loadedPalette;
      return context()->throwError(
          tr("Exception reading palette %1").arg(pathArg.toString()));
    }
  }

  // Color management (solid colors)
  Q_INVOKABLE QScriptValue addColor(int r, int g, int b, int a = 255);
  Q_INVOKABLE QScriptValue setStyleColor(int styleIdx, int r, int g, int b,
                                         int a = 255);
  Q_INVOKABLE QScriptValue getStyleColor(int styleIdx);

  // Style system (gradients, patterns, textures, decorative strokes, etc.)
  Q_INVOKABLE QScriptValue addStyle(int tagId);
  Q_INVOKABLE QScriptValue getStyleType(int styleIdx);
  Q_INVOKABLE QScriptValue getStyleParamCount(int styleIdx);
  Q_INVOKABLE QScriptValue getStyleParamNames(int styleIdx);
  Q_INVOKABLE QScriptValue setStyleParam(int styleIdx, int paramIdx,
                                         const QScriptValue &value);
  Q_INVOKABLE QScriptValue getStyleParam(int styleIdx, int paramIdx);
  Q_INVOKABLE QScriptValue getStyleColorParamCount(int styleIdx);
  Q_INVOKABLE QScriptValue setStyleColorParam(int styleIdx, int colorIdx,
                                              int r, int g, int b,
                                              int a = 255);
  Q_INVOKABLE QScriptValue getStyleColorParam(int styleIdx, int colorIdx);
  Q_INVOKABLE QScriptValue getAvailableTags();

  // Color model (reference image)
  Q_INVOKABLE QScriptValue loadColorModel(const QScriptValue &pathArg);
  Q_INVOKABLE QScriptValue pickColorFromModel(int x, int y);
  Q_INVOKABLE QScriptValue removeColorModel();

  // Page management
  Q_INVOKABLE QScriptValue addPage(const QString &name);

  // Properties
  Q_PROPERTY(int styleCount READ getStyleCount)
  int getStyleCount() const;

  Q_PROPERTY(int pageCount READ getPageCount)
  int getPageCount() const;

  // Access underlying
  TPalette *getPalette() const { return m_palette; }
};

QScriptValue checkPalette(QScriptContext *context, const QScriptValue &value,
                          Palette *&out);

}  // namespace TScriptBinding

Q_DECLARE_METATYPE(TScriptBinding::Palette *)

#endif
