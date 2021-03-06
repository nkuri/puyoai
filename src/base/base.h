#ifndef BASE_BASE_H_
#define BASE_BASE_H_

#define ARRAY_SIZE(a)                                   \
    ((sizeof(a) / sizeof(*(a))) /                       \
     static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

#define CLEAR_ARRAY(a) memset(a, 0, sizeof(a))

#define UNUSED_VARIABLE(x) (void)(x)

#ifndef __has_feature
#  define __has_feature(x) 0
#endif

#ifndef __has_extension
#  define __has_extension(x) 0
#endif

#if __has_extension(attribute_deprecated_with_message)
#  define DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#  define DEPRECATED __attribute__((deprecated()))
#else
#  define DEPRECATED_MSG(msg)
#  define DEPRECATED
#endif

#endif  // BASE_BASE_H_
