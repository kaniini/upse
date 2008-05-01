int upse_ps1_spu_init(void);
int upse_ps1_spu_open(void);
void upse_ps1_spu_setlength(s32 stop, s32 fade);
int upse_ps1_spu_close(void);
void upse_ps1_spu_finalize(void);

// External, called by SPU code.
void SPUirq(void);
void upse_ps1_spu_setvolume(int volume);
