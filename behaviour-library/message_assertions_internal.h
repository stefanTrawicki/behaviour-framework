#ifndef MESSAGE_ASSERTIONS_INTERNAL_H
#define MESSAGE_ASSERTIONS_INTERNAL_H

#define ASSERT_MSG(COND, MSG) ({if(COND){printf(MSG" (%s:%d)\n", __FILE__, __LINE__); exit(EXIT_FAILURE);} })

#endif // !MESSAGE_ASSERTIONS_INTERNAL_H