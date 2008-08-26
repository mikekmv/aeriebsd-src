
int	pckbd_cnattach(pckbc_tag_t, int);
void	pckbd_hookup_bell(void (*fn)(void *, u_int, u_int, u_int, int),
	    void *);
