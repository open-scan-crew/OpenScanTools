#ifndef AUTHOR_TEXTS_H
#define AUTHOR_TEXTS_H

#include <QtCore/qobject.h>

//AuthorSystem
#define TEXT_WARNING_AUTHOR_SELECT QObject::tr("Error : cannot select user : unknow error.") //author renommé en user pour cohérence avec panneau
#define TEXT_WARNING_AUTHOR_CREATE QObject::tr("Error : cannot create a user that already exists.") //author renommé en user pour cohérence avec panneau
#define TEXT_WARNING_AUTHOR_DELETE QObject::tr("Error : cannot delete user : unknow error.") //author renommé en user pour cohérence avec panneau
#define TEXT_WARNING_AUTHOR_WRONG_CLOSE QObject::tr("Error : you must select a user in order to continue.") //author renommé en user pour cohérence avec panneau
#define TEXT_WARNING_AUTHOR_SELECT_ONE QObject::tr("You must select an author.")
#define TEXT_DELETE_AUTHOR_QUESTION QObject::tr("Are you sure that you want to delete the selected user(s)?") //author renommé en user pour cohérence avec panneau
#define TEXT_DELETE_AUTHOR_TITLE QObject::tr("Delete user(s)?") //author renommé en user pour cohérence avec panneau

#endif