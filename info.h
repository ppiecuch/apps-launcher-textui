#ifndef INFO_H
#define INFO_H

#define APP_VERSION       1.05
#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 0.05

#ifdef DEBUG_BUILD
#define APP_FULL_IDENT    1.05[DEBUG]
#else
#define APP_FULL_IDENT    1.05[PROD]
#endif

#define BUILDTIME __DATE__ "/" __TIME__

#endif // INFO_H
