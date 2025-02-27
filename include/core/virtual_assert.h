#ifndef _VIRTUAL_ASSERT_H_
#define _VIRTUAL_ASSERT_H_

#define virtual_os_assert(cond)                                                                                        \
	do {                                                                                                               \
		if (!(cond)) {                                                                                                 \
			while (1)                                                                                                  \
				;                                                                                                      \
		}                                                                                                              \
	} while (0)

#endif /* _VIRTUAL_ASSERT_H_ */