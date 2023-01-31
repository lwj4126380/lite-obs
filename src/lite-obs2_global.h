#ifndef LITEOBS2_GLOBAL_H
#define LITEOBS2_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LITEOBS2_LIBRARY)
#  define LITEOBS2_EXPORT Q_DECL_EXPORT
#else
#  define LITEOBS2_EXPORT Q_DECL_IMPORT
#endif

#endif // LITEOBS2_GLOBAL_H
