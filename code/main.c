#include "spp_sol.h"
#include "io.h"

extern opt_file_t      opt_file = { 0 };
extern files_manager_t files_manager;

static void output_pos_file_header(void)
{
    if (!files_manager.out_pos.is_open)
    {
        return;
    }

    FILE *fp = files_manager.out_pos.fp;
    fprintf(fp, "%% program   : Simple_SPP ver.1.0\n");
    fprintf(fp, "%% inp file  : %s\n%% inp file  : %s\n", opt_file.obs_file, opt_file.nav_file);
    fprintf(fp, "%% obs start : 2020/08/09 %02d:%02d:%04.1f\n", (int32_t)opt_file.start_time[0],
            (int32_t)opt_file.start_time[1], opt_file.start_time[2]);
    fprintf(fp, "%% obs start : 2020/08/09 %02d:%02d:%04.1f\n", (int32_t)opt_file.end_time[0],
            (int32_t)opt_file.end_time[1], opt_file.end_time[2]);
    fprintf(fp, "%%\n%% (x/y/z-ecef=WGS84,Q=1:fix,2:float,3:sbas,4:dgps,5:single,6:ppp,ns=# of satellites)\n");
    fprintf(fp,"%%  GPST                      x-ecef(m)      y-ecef(m)      z-ecef(m)   Q  ns   sdx(m)   sdy(m)   sdz(m)  sdxy(m)  sdyz(m)  sdzx(m) age(s)  ratio)\n");
    fflush(fp);
}

RETURN_STATUS main(int32_t args, char *argv[])

{
    /* read option file */
    RETURN_STATUS ret_status = RET_FAIL;
    ret_status = read_option_file(&opt_file, args, argv[1]);
    /* creat log file */
    open_files();
    output_pos_file_header();

    if (!ret_status)
    {   
        close_files();

        //return RET_FAIL;
    }
    ret_status = proc(&opt_file);

    close_files();

    return RET_SUCCESS;
}