/* ./basic.c */
extern int F_beginning_of_line(int uniarg);
extern int F_end_of_line(int uniarg);
extern int previous_line(void);
extern int F_previous_line(int uniarg);
extern int next_line(void);
extern int F_next_line(int uniarg);
extern void gotobob(void);
extern int F_beginning_of_buffer(int uniarg);
extern void gotoeob(void);
extern int F_end_of_buffer(int uniarg);
extern int backward_char(void);
extern int F_backward_char(int uniarg);
extern int forward_char(void);
extern int F_forward_char(int uniarg);
extern int ngotoup(int n);
extern int ngotodown(int n);
extern int scroll_down(void);
extern int F_scroll_down(int uniarg);
extern int scroll_up(void);
extern int F_scroll_up(int uniarg);
extern void goto_line(int to_line);
extern int F_goto_line(int uniarg);
/* ./bind.c */
extern void bind_key(char *key, funcp func);
extern int do_completion(char *s, int *compl);
extern void process_key(int c);
extern struct fentry fentry_table[];
extern void init_bindings(void);
extern void free_bindings(void);
extern int execute_function(char *name, int uniarg);
extern int F_execute_extended_command(int uniarg);
extern char *minibuf_read_function_name(char *msg);
extern char *get_function_by_key_sequence(void);
extern int F_list_functions(int uniarg);
extern int F_list_bindings(int uniarg);
/* ./buffer.c */
extern bufferp new_buffer(void);
extern void free_buffer(bufferp bp);
extern void free_buffers(void);
extern bufferp create_buffer(char *name);
extern void set_buffer_name(bufferp bp, char *name);
extern void set_buffer_filename(bufferp bp, char *filename);
extern bufferp find_buffer(char *name, int cflag);
extern bufferp get_next_buffer(void);
extern char *make_buffer_name(char *filename);
extern void switch_to_buffer(bufferp bp);
extern int zap_buffer_content(void);
extern int warn_if_readonly_buffer(void);
extern int warn_if_no_mark(void);
extern int calculate_region(regionp rp);
extern void set_temporary_buffer(bufferp bp);
/* ./file.c */
extern int exist_file(char *filename);
extern int is_regular_file(char *filename);
extern int expand_path(char *path, char *cwdir, char *dir, char *fname);
extern char *compact_path(char *buf, char *path);
extern char *get_current_dir(char *buf);
extern void open_file(char *path, int lineno);
extern void read_from_disk(char *filename);
extern int find_file(char *filename);
extern historyp make_buffer_history(void);
extern int F_find_file(int uniarg);
extern int F_find_alternate_file(int uniarg);
extern int F_switch_to_buffer(int uniarg);
extern int check_modified_buffer(bufferp bp);
extern void kill_buffer(bufferp kill_bp);
extern int F_kill_buffer(int uniarg);
extern int F_insert_buffer(int uniarg);
extern int F_insert_file(int uniarg);
extern int F_save_buffer(int uniarg);
extern int F_write_file(int uniarg);
extern int F_save_some_buffers(int uniarg);
extern int F_save_buffers_kill_zile(int uniarg);
extern int F_cd(int uniarg);
/* ./fontlock.c */
extern void font_lock_reset_anchors(bufferp bp, linep lp);
extern int find_last_anchor(bufferp bp, linep lp);
extern int F_font_lock_mode(int uniarg);
extern int F_font_lock_refresh(int uniarg);
/* ./fontlock_c.c */
extern char *is_c_keyword(const char *str, int len);
/* ./fontlock_cpp.c */
extern char *is_cpp_keyword(const char *str, int len);
/* ./funcs.c */
extern int cancel(void);
extern int F_suspend_zile(int uniarg);
extern int F_keyboard_quit(int uniarg);
extern int F_list_buffers(int uniarg);
extern int F_overwrite_mode(int uniarg);
extern int F_toggle_read_only(int uniarg);
extern int F_auto_fill_mode(int uniarg);
extern int F_text_mode(int uniarg);
extern int F_c_mode(int uniarg);
extern int F_cpp_mode(int uniarg);
extern int set_mark_command(void);
extern int F_set_mark_command(int uniarg);
extern int F_exchange_point_and_mark(int uniarg);
extern int F_mark_whole_buffer(int uniarg);
extern int F_upcase_region(int uniarg);
extern int F_downcase_region(int uniarg);
extern int F_quoted_insert(int uniarg);
extern int universal_argument(int keytype, int xarg);
extern int F_universal_argument(int uniarg);
extern int F_set_fill_column(int uniarg);
extern void write_to_temporary_buffer(char *name, void (*write_text)(void *, void *, void *, void *), void *p1, void *p2, void *p3, void *p4);
extern int F_tabify(int uniarg);
extern int F_untabify(int uniarg);
/* ./glue.c */
extern void ding(void);
extern void waitkey(int msecs);
extern int waitkey_discard(int msecs);
extern char *copy_text_block(int startn, int starto, size_t size);
extern char *shorten_string(char *dest, char *s, int maxlen);
extern char *replace_string(char *s, char *match, char *subst);
extern void tabify_string(char *dest, char *src, int scol, int tw);
extern void untabify_string(char *dest, char *src, int scol, int tw);
extern int get_text_goalc(windowp wp);
int calculate_mark_lineno(windowp wp);
/* ./help.c */
extern int F_zile_version(int uniarg);
extern int F_toggle_minihelp_window(int uniarg);
extern int F_help_latest_version(int uniarg);
extern int F_help(int uniarg);
extern int F_help_config_sample(int uniarg);
extern int F_help_faq(int uniarg);
extern int F_help_tutorial(int uniarg);
extern int F_describe_function(int uniarg);
extern int F_describe_variable(int uniarg);
extern int F_describe_key(int uniarg);
/* ./keys.c */
extern char *keytostr(char *buf, int key, int *len);
extern char *keytostr_nobs(char *buf, int key, int *len);
extern int strtokey(char *buf, int *len);
extern int keytovec(char *key, int *keyvec);
extern char *simplify_key(char *dest, char *key);
/* ./line.c */
extern linep new_line(int maxsize);
extern linep resize_line(windowp wp, linep lp, int maxsize);
extern void free_line(linep lp);
extern void line_replace_text(linep *lp, int offset, int orgsize, char *newtext);
extern int insert_char(int c);
extern int insert_tab(void);
extern int insert_newline(void);
extern void insert_string(char *s);
extern void insert_nstring(char *s, size_t size);
extern int self_insert_command(int c);
extern int F_self_insert_command(int uniarg);
extern void bprintf(const char *fmt, ...);
extern int delete_char(void);
extern int F_delete_char(int uniarg);
extern int backward_delete_char(void);
extern int F_backward_delete_char(int uniarg);
extern int F_kill_line(int uniarg);
extern int F_copy_region_as_kill(int uniarg);
extern int F_kill_region(int uniarg);
extern int F_yank(int uniarg);
extern int F_copy_to_register(int uniarg);
extern int F_insert_register(int uniarg);
extern int F_newline(int uniarg);
extern int F_tab_to_tab_stop(int uniarg);
extern int F_list_registers(int uniarg);
extern void free_registers(void);
extern void free_kill_ring(void);
/* ./macro.c */
extern void cancel_kbd_macro(void);
extern void add_kbd_macro(funcp func, int uniarg);
extern int F_start_kbd_macro(int uniarg);
extern int F_end_kbd_macro(int uniarg);
extern int F_call_last_kbd_macro(int uniarg);
extern void free_macros(void);
/* ./main.c */
extern windowp cur_wp;
extern windowp head_wp;
extern bufferp cur_bp;
extern bufferp prev_bp;
extern bufferp head_bp;
extern terminalp cur_tp;
extern int thisflag;
extern int lastflag;
extern int last_uniarg;
extern int main(int argc, char **argv);
/* ./minibuf.c */
extern void minibuf_write(const char *fmt, ...);
extern void minibuf_error(const char *fmt, ...);
extern char *minibuf_read(const char *fmt, char *value, ...);
extern char *minibuf_read_dir(const char *fmt, char *value, ...);
extern int minibuf_read_yesno(const char *fmt, ...);
extern int minibuf_read_boolean(const char *fmt, ...);
extern char *minibuf_read_color(const char *fmt, ...);
extern char *minibuf_read_history(const char *fmt, char *value, historyp hp, ...);
extern void minibuf_clear(void);
extern historyp new_history(int size, int fileflag);
extern void free_history(historyp hp);
/* ./rc.c */
extern void read_rc_file(void);
/* ./redisplay.c */
extern void resync_redisplay(void);
extern void recenter(windowp wp);
extern int F_recenter(int uniarg);
/* ./search.c */
extern int F_search_forward(int uniarg);
extern int F_search_backward(int uniarg);
extern void free_search_history(void);
/* ./undo.c */
extern int undo_nosave;
extern void undo_save(int type, int startn, int starto, int arg1, int arg2);
extern int F_undo(int uniarg);
/* ./variables.c */
extern void init_variables(void);
extern void free_variables(void);
extern void set_variable(char *var, char *val);
extern void unset_variable(char *var);
extern char *get_variable(char *var);
extern int is_variable_equal(char *var, char *val);
extern int lookup_bool_variable(char *var);
extern int F_set_variable(int uniarg);
extern int F_list_variables(int uniarg);
char *minibuf_read_variable_name(char *msg);
/* ./window.c */
extern windowp new_window(void);
extern void free_window(windowp wp);
extern void free_windows(void);
extern int F_split_window(int uniarg);
extern int F_delete_window(int uniarg);
extern int F_enlarge_window(int uniarg);
extern windowp popup_window(void);
extern int F_delete_other_windows(int uniarg);
extern int F_other_window(int uniarg);
extern void create_first_window(void);
extern windowp find_window(char *name);
