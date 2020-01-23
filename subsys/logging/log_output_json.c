
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <data/json.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <logging/log_output.h>

static u32_t level_to_syst_severity(u32_t level)
{
	u32_t ret;

	switch (level) {
	case LOG_LEVEL_NONE:
		ret = 0U;
		break;
	case LOG_LEVEL_ERR:
		ret = 1U;
		break;
	case LOG_LEVEL_WRN:
		ret = 2U;
		break;
	case LOG_LEVEL_INF:
		ret = 3U;
		break;
	case LOG_LEVEL_DBG:
		ret = 4U;
		break;
	default:
		ret = 5U;
		break;
	}

	return ret;
}

static void std_print(struct log_msg *msg,
		      const struct log_output *log_output)
{
	const char *str = log_msg_str_get(msg);
	u32_t nargs = log_msg_nargs_get(msg);
	u32_t *args = alloca(sizeof(u32_t)*nargs);
	int i;

	for (i = 0; i < nargs; i++) {
		args[i] = log_msg_arg_get(msg, i);
	}

	switch (log_msg_nargs_get(msg)) {
	case 0:
		print_formatted(log_output, str);
		break;
	case 1:
		print_formatted(log_output, str, args[0]);
		break;
	case 2:
		print_formatted(log_output, str, args[0], args[1]);
		break;
	case 3:
		print_formatted(log_output, str, args[0], args[1], args[2]);
		break;
	case 4:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3]);
		break;
	case 5:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4]);
		break;
	case 6:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5]);
		break;
	case 7:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5], args[6]);
		break;
	case 8:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5], args[6], args[7]);
		break;
	case 9:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5], args[6],  args[7],
				args[8]);
		break;
	case 10:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5], args[6],  args[7],
				args[8], args[9]);
		break;
	case 11:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5], args[6],  args[7],
				args[8], args[9], args[10]);
		break;
	case 12:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5], args[6],  args[7],
				args[8], args[9], args[10], args[11]);
		break;
	case 13:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5], args[6],  args[7],
				args[8], args[9], args[10], args[11], args[12]);
		break;
	case 14:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5], args[6],  args[7],
				args[8], args[9], args[10], args[11], args[12],
				args[13]);
		break;
	case 15:
		print_formatted(log_output, str, args[0], args[1], args[2],
				args[3], args[4], args[5], args[6],  args[7],
				args[8], args[9], args[10], args[11], args[12],
				args[13], args[14]);
		break;
	default:
		/* Unsupported number of arguments. */
		__ASSERT_NO_MSG(true);
		break;
	}
}


static int print_formatted(const struct log_output *log_output,
			   const char *fmt, ...)
{
	va_list args;
	int length = 0;

	va_start(args, fmt);
#if !defined(CONFIG_NEWLIB_LIBC) && !defined(CONFIG_ARCH_POSIX) && \
    defined(CONFIG_LOG_ENABLE_FANCY_OUTPUT_FORMATTING)
	length = z_prf(out_func, (void *)log_output, (char *)fmt, args);
	length =
#else
	z_vprintk(out_func, (void *)log_output, fmt, args);
#endif
	va_end(args);

	return length;
}
