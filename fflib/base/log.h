
#ifndef _FF_LOG_H_
#define _FF_LOG_H_

#include <unistd.h>
#include <sys/syscall.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <set>
#include <vector>
#include <fstream>

#include "base/task_queue.h"
#include "base/thread.h"
#include "base/singleton.h"
#include "base/arg_helper.h"

#define gettid() 0 // ::syscall(SYS_gettid)

namespace ff
{

class StrFormat
{
	struct FmtType
	{
		FmtType():
			type('\0'),
			min_len(0),
			fill_char(' ')
		{}
		void clear()
		{
			type = '\0';
			min_len = 0;
			fill_char = ' ';
		}
		char 			type;//! % d,x,f,s,
		unsigned int 	min_len;
		char            fill_char;
	};
public:
	//! fmt_ like "xxx%d,xx%s"
	StrFormat(const char* fmt_ = "");
	virtual ~StrFormat();

	template<typename T>
	void append(T content_)
	{
		if (moveToNextWildcard())
		{
			if (m_fmt_type.type == 'x')
			{
				char buff[64];
				snprintf(buff, sizeof(buff), "0x%lx", (unsigned long)content_);
				m_num_buff = buff;
			}
			else
			{
				m_strstream << content_;
				m_strstream >> m_num_buff;
			}
			int width = m_fmt_type.min_len > m_num_buff.length()? m_fmt_type.min_len - m_num_buff.length(): 0;
			for (; width > 0; -- width)
			{
				m_result += m_fmt_type.fill_char;
			}
		}
		else
		{
			m_strstream << content_;
			m_strstream >> m_num_buff;
		}

		m_result += m_num_buff;
		m_strstream.clear();//! clear error bit,not content
		m_num_buff.clear();
	}
	void append(const char* str_);
	void append(const std::string& str_);
	const std::string& genResult();
private:
	bool moveToNextWildcard();

protected:
	const char*     	 m_fmt;
	unsigned int         cur_format_index;
	unsigned int    	 m_fmt_len;
	FmtType              m_fmt_type;
	std::string      	 m_result;
	std::stringstream    m_strstream;
	std::string          m_num_buff;
};

enum log_level_e
{
	LOG_FATAL = 0,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_TRACE,
	LOG_DEBUG,
	LOG_LEVEL_NUM
};

class Log
{
	enum log_e
	{
		MAX_LINE_NUM = 5000
	};

public:
	Log(int level_, const std::string& all_class_, const std::string& path_, const std::string& file_,
		  bool print_file_, bool print_screen_);
	virtual ~Log();
	void setPathAndName(const std::string& path, const std::string& name);
	void setLevel(int lv);
	void setLevel(int level_, bool flag_);
	void setModule(const std::string& class_, bool flag_);
	void setPrintFile(bool flag_);
	void setPrintScreen(bool flag_);
	bool is_level_enabled(int level_);
	const char* find_class_name(const char* class_);

	void log_content(int level_, const char* str_class_, const std::string& content_, long tid_);

protected:
	bool check_and_create_dir(struct tm* tm_val_);

protected:
	int 						m_enabled_level;
	typedef std::set<std::string>			str_set_t;
	typedef std::vector<str_set_t*>	ptr_vt_t;
	str_set_t*					m_enable_class_set;
	ptr_vt_t					m_class_set_history;

	struct tm					m_last_create_dir_tm;
	bool						m_enable_file;
	bool             			m_enable_screen;

	std::ofstream 					m_file;
	std::string                      m_path;
	std::string                      m_filename;
	unsigned int                m_file_name_index;
	unsigned int                m_line_num;
};

#define LOG_IMPL_NONE_ARG(func, LOG_LEVEL) 															\
	void func(const char* class_, const char* fmt_)													\
	{																								\
		if (m_log->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = m_log->find_class_name(class_);							\
			if (m_bEnableAllClass || class_name_str)																		\
			{																						\
				m_task_queue.post(funcbind(&Log::log_content, m_log, LOG_LEVEL,		\
									 class_name_str?class_name_str:class_, std::string(fmt_), gettid()));								\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG1(func, LOG_LEVEL) 																\
	template <typename ARG1>																		\
	void func(const char* class_, const char* fmt_, const ARG1& arg1_)								\
	{																								\
		if (m_log->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = m_log->find_class_name(class_);							\
			if (m_bEnableAllClass || class_name_str)																		\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				m_task_queue.post(funcbind(&Log::log_content, m_log, LOG_LEVEL,		\
									 class_name_str?class_name_str:class_, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}


#define LOG_IMPL_ARG2(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2>															\
	void func(const char* class_, const char* fmt_, const ARG1& arg1_, const ARG2& arg2_)			\
	{																								\
		if (m_log->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = m_log->find_class_name(class_);							\
			if (m_bEnableAllClass || class_name_str)																		\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				m_task_queue.post(funcbind(&Log::log_content, m_log, LOG_LEVEL,		\
									 class_name_str?class_name_str:class_, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG3(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2, typename ARG3>											\
	void func(const char* class_, const char* fmt_, const ARG1& arg1_, const ARG2& arg2_,			\
			  const ARG3& arg3_)																	\
	{																								\
		if (m_log->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = m_log->find_class_name(class_);							\
			if (m_bEnableAllClass || class_name_str)																		\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				dest.append(arg3_);																	\
				m_task_queue.post(funcbind(&Log::log_content, m_log, LOG_LEVEL,		\
									 class_name_str?class_name_str:class_, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG4(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2, typename ARG3, typename ARG4>							\
	void func(const char* class_, const char* fmt_, const ARG1& arg1_, const ARG2& arg2_,			\
			  const ARG3& arg3_, const ARG4& arg4_)													\
	{																								\
		if (m_log->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = m_log->find_class_name(class_);							\
			if (m_bEnableAllClass || class_name_str)																		\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				dest.append(arg3_);																	\
				dest.append(arg4_);																	\
				m_task_queue.post(funcbind(&Log::log_content, m_log, LOG_LEVEL,		\
									 class_name_str?class_name_str:class_, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG5(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>			\
	void func(const char* class_, const char* fmt_, const ARG1& arg1_, const ARG2& arg2_,			\
			  const ARG3& arg3_, const ARG4& arg4_, const ARG5& arg5_)								\
	{																								\
		if (m_log->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = m_log->find_class_name(class_);							\
			if (m_bEnableAllClass || class_name_str)																		\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				dest.append(arg3_);																	\
				dest.append(arg4_);																	\
				dest.append(arg5_);																	\
				m_task_queue.post(funcbind(&Log::log_content, m_log, LOG_LEVEL,		\
									 class_name_str?class_name_str:class_, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG6(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,			\
			  typename ARG6>																		\
	void func(const char* class_, const char* fmt_, const ARG1& arg1_, const ARG2& arg2_,			\
			  const ARG3& arg3_, const ARG4& arg4_, const ARG5& arg5_, const ARG6& arg6_)			\
	{																								\
		if (m_log->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = m_log->find_class_name(class_);							\
			if (m_bEnableAllClass || class_name_str)																		\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				dest.append(arg3_);																	\
				dest.append(arg4_);																	\
				dest.append(arg5_);																	\
				dest.append(arg6_);																	\
				m_task_queue.post(funcbind(&Log::log_content, m_log, LOG_LEVEL,		\
									 class_name_str?class_name_str:class_, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}

#define LOG_IMPL_MACRO(asyncLogdebug, LOG_DEBUG) 	\
	LOG_IMPL_NONE_ARG(asyncLogdebug, LOG_DEBUG)  	\
	LOG_IMPL_ARG1(asyncLogdebug, LOG_DEBUG)	  	\
	LOG_IMPL_ARG2(asyncLogdebug, LOG_DEBUG)	  	\
	LOG_IMPL_ARG3(asyncLogdebug, LOG_DEBUG)		\
	LOG_IMPL_ARG4(asyncLogdebug, LOG_DEBUG)		\
	LOG_IMPL_ARG5(asyncLogdebug, LOG_DEBUG)		\
	LOG_IMPL_ARG6(asyncLogdebug, LOG_DEBUG)

class LogService
{
public:
	LogService();
	~LogService();
	int start(const std::string& opt_);
    int start(ArgHelper& arg_helper);
	int stop();

	LOG_IMPL_MACRO(asyncLogdebug, LOG_DEBUG);
	LOG_IMPL_MACRO(asyncLogtrace, LOG_TRACE);
	LOG_IMPL_MACRO(asyncLoginfo,  LOG_INFO);
	LOG_IMPL_MACRO(asyncLogwarn,  LOG_WARN);
	LOG_IMPL_MACRO(asyncLogerror, LOG_ERROR);
	LOG_IMPL_MACRO(asyncLogfatal, LOG_FATAL);

	void setLevel(int level_, bool flag_);
	void setModule(const std::string& class_, bool flag_);
	void setPrintFile(bool flag_);
	void setPrintScreen(bool flag_);
	void setEnableAllClass(bool f) { m_bEnableAllClass = f;}

	Log* getLogObj() { return m_log; }
	TaskQueue& getTQ() { return m_task_queue; }
private:
	Log*			m_log;
	TaskQueue       m_task_queue;
	bool            m_bEnableAllClass;
};

#define BROKER  "BROKER"
#define RPC     "RPC"
#define FF      "FF"
#define MSG_BUS "MSG_BUS"

#define LOG Singleton<LogService>::instance()
#define LOGDEBUG(content)  Singleton<LogService>::instance().asyncLogdebug content
#define LOGTRACE(content)  Singleton<LogService>::instance().asyncLogtrace content
#define LOGINFO(content)   Singleton<LogService>::instance().asyncLoginfo  content
#define LOGWARN(content)   Singleton<LogService>::instance().asyncLogwarn  content
#define LOGERROR(content)  Singleton<LogService>::instance().asyncLogerror content
#define LOGFATAL(content)  Singleton<LogService>::instance().asyncLogfatal content

#define LOG_IMPL_NONE_ARG_NOCLASS(func, LOG_LEVEL) 															\
	static void func(const char* fmt_)													\
	{																								\
		if (Singleton<LogService>::instance().getLogObj()->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = "SERVER";							\
			{																						\
				Singleton<LogService>::instance().getTQ().post(funcbind(&Log::log_content, Singleton<LogService>::instance().getLogObj(), LOG_LEVEL,		\
									 class_name_str, std::string(fmt_), gettid()));								\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG_NOCLASS1(func, LOG_LEVEL) 																\
	template <typename ARG1>																		\
	static void func(const char* fmt_, const ARG1& arg1_)								\
	{																								\
		if (Singleton<LogService>::instance().getLogObj()->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = "SERVER";							\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				Singleton<LogService>::instance().getTQ().post(funcbind(&Log::log_content, Singleton<LogService>::instance().getLogObj(), LOG_LEVEL,		\
									 class_name_str, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}


#define LOG_IMPL_ARG_NOCLASS2(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2>															\
	static void func(const char* fmt_, const ARG1& arg1_, const ARG2& arg2_)			\
	{																								\
		if (Singleton<LogService>::instance().getLogObj()->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = "SERVER";							\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				Singleton<LogService>::instance().getTQ().post(funcbind(&Log::log_content, Singleton<LogService>::instance().getLogObj(), LOG_LEVEL,		\
									 class_name_str, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG_NOCLASS3(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2, typename ARG3>											\
	static void func(const char* fmt_, const ARG1& arg1_, const ARG2& arg2_,			\
			  const ARG3& arg3_)																	\
	{																								\
		if (Singleton<LogService>::instance().getLogObj()->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = "SERVER";							\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				dest.append(arg3_);																	\
				Singleton<LogService>::instance().getTQ().post(funcbind(&Log::log_content, Singleton<LogService>::instance().getLogObj(), LOG_LEVEL,		\
									 class_name_str, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG_NOCLASS4(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2, typename ARG3, typename ARG4>							\
	static void func(const char* fmt_, const ARG1& arg1_, const ARG2& arg2_,			\
			  const ARG3& arg3_, const ARG4& arg4_)													\
	{																								\
		if (Singleton<LogService>::instance().getLogObj()->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = "SERVER";							\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				dest.append(arg3_);																	\
				dest.append(arg4_);																	\
				Singleton<LogService>::instance().getTQ().post(funcbind(&Log::log_content, Singleton<LogService>::instance().getLogObj(), LOG_LEVEL,		\
									 class_name_str, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG_NOCLASS5(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>			\
	static void func(const char* fmt_, const ARG1& arg1_, const ARG2& arg2_,			\
			  const ARG3& arg3_, const ARG4& arg4_, const ARG5& arg5_)								\
	{																								\
		if (Singleton<LogService>::instance().getLogObj()->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = "SERVER";							\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				dest.append(arg3_);																	\
				dest.append(arg4_);																	\
				dest.append(arg5_);																	\
				Singleton<LogService>::instance().getTQ().post(funcbind(&Log::log_content, Singleton<LogService>::instance().getLogObj(), LOG_LEVEL,		\
									 class_name_str, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}

#define LOG_IMPL_ARG_NOCLASS6(func, LOG_LEVEL) 																\
	template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5,			\
			  typename ARG6>																		\
	static void func(const char* fmt_, const ARG1& arg1_, const ARG2& arg2_,			\
			  const ARG3& arg3_, const ARG4& arg4_, const ARG5& arg5_, const ARG6& arg6_)			\
	{																								\
		if (Singleton<LogService>::instance().getLogObj()->is_level_enabled(LOG_LEVEL))														\
		{																							\
			const char* class_name_str = "SERVER";							\
			{																						\
				StrFormat dest(fmt_);															\
				dest.append(arg1_);																	\
				dest.append(arg2_);																	\
				dest.append(arg3_);																	\
				dest.append(arg4_);																	\
				dest.append(arg5_);																	\
				dest.append(arg6_);																	\
				Singleton<LogService>::instance().getTQ().post(funcbind(&Log::log_content, Singleton<LogService>::instance().getLogObj(), LOG_LEVEL,		\
									 class_name_str, dest.genResult(), gettid()));						 	\
			}																						\
		}																							\
	}


#define LOG_IMPL_MACRO_NOCLASS(funcname, LOG_LV) 	\
	LOG_IMPL_NONE_ARG_NOCLASS(funcname, LOG_LV)  	\
	LOG_IMPL_ARG_NOCLASS1(funcname, LOG_LV)	  	\
	LOG_IMPL_ARG_NOCLASS2(funcname, LOG_LV)	  	\
	LOG_IMPL_ARG_NOCLASS3(funcname, LOG_LV)		\
	LOG_IMPL_ARG_NOCLASS4(funcname, LOG_LV)		\
	LOG_IMPL_ARG_NOCLASS5(funcname, LOG_LV)		\
	LOG_IMPL_ARG_NOCLASS6(funcname, LOG_LV)

struct LogUtil{
	LOG_IMPL_MACRO_NOCLASS(logdebugUtil, LOG_DEBUG);
	LOG_IMPL_MACRO_NOCLASS(logtraceUtil, LOG_TRACE);
	LOG_IMPL_MACRO_NOCLASS(loginfoUtil , LOG_INFO) ;
	LOG_IMPL_MACRO_NOCLASS(logwarnUtil , LOG_WARN) ;
	LOG_IMPL_MACRO_NOCLASS(logerrorUtil, LOG_ERROR);
	LOG_IMPL_MACRO_NOCLASS(logfatalUtil, LOG_FATAL);
};

#define logdebug LogUtil::logdebugUtil
#define logtrace LogUtil::logtraceUtil
#define loginfo  LogUtil::loginfoUtil
#define logwarn  LogUtil::logwarnUtil
#define logerror LogUtil::logerrorUtil
#define logfatal LogUtil::logfatalUtil

}

#endif


