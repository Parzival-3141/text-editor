#ifndef ERR_UTILS_H_
#define ERR_UTILS_H_

void CheckSDLError(int code);
void* CheckSDLPtr(void* ptr);
void CheckFTError(int code, const char* err_msg);
void* CheckPtr(void* ptr, const char* msg_format, ...);

void check_gl_err(void);

#endif // ERR_UTILS_H_
