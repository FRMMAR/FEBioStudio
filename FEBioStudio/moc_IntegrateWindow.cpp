/****************************************************************************
** Meta object code from reading C++ file 'IntegrateWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "IntegrateWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IntegrateWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CIntegrateWindow_t {
    QByteArrayData data[1];
    char stringdata0[17];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CIntegrateWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CIntegrateWindow_t qt_meta_stringdata_CIntegrateWindow = {
    {
QT_MOC_LITERAL(0, 0, 16) // "CIntegrateWindow"

    },
    "CIntegrateWindow"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CIntegrateWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void CIntegrateWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CIntegrateWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<CGraphWindow::staticMetaObject>(),
    qt_meta_stringdata_CIntegrateWindow.data,
    qt_meta_data_CIntegrateWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CIntegrateWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CIntegrateWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CIntegrateWindow.stringdata0))
        return static_cast<void*>(this);
    return CGraphWindow::qt_metacast(_clname);
}

int CIntegrateWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CGraphWindow::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_CIntegrateSurfaceWindow_t {
    QByteArrayData data[1];
    char stringdata0[24];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CIntegrateSurfaceWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CIntegrateSurfaceWindow_t qt_meta_stringdata_CIntegrateSurfaceWindow = {
    {
QT_MOC_LITERAL(0, 0, 23) // "CIntegrateSurfaceWindow"

    },
    "CIntegrateSurfaceWindow"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CIntegrateSurfaceWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void CIntegrateSurfaceWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CIntegrateSurfaceWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<CGraphWindow::staticMetaObject>(),
    qt_meta_stringdata_CIntegrateSurfaceWindow.data,
    qt_meta_data_CIntegrateSurfaceWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CIntegrateSurfaceWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CIntegrateSurfaceWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CIntegrateSurfaceWindow.stringdata0))
        return static_cast<void*>(this);
    return CGraphWindow::qt_metacast(_clname);
}

int CIntegrateSurfaceWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CGraphWindow::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
