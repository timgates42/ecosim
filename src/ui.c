#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "keyboard.h"
#define MAX_BUFF_LEN (64)

Ui*
ui_setup(void)
{
  Ui* tmp = malloc(sizeof(Ui));
  float tmp_sel[] = {-0.1f, -0.5f, 0.0f, 0.0f};

  tmp->mode = UI_MODE_NORM;
  tmp->buff = malloc(sizeof(char) * MAX_BUFF_LEN);
  tmp->buff_len = 0, tmp->last_out_msg = 0;
  memcpy(tmp->selection, tmp_sel, sizeof(tmp_sel));
  strcpy(tmp->buff, "\0");

  return tmp;
}

/* Return ui response */
Ui_resp*
ui_get_resp(Ui* ui, Keyboard_event* key_ev)
{
  Ui_resp* resp = malloc(sizeof(Ui_resp));

  if(key_ev->special){
    if(key_ev->special == RETURN)
    {
      ui->mode = !ui->mode;
      printf("mode %d\n", ui->mode);
    }
  }
  switch(ui->mode){
    case UI_MODE_NORM:
      ui_resp_norm_mode(ui, key_ev, resp);
      break;
    case UI_MODE_SELECT:
      ui_resp_sel_mode(ui, key_ev, resp);
      break;
  };
  return resp;
}

/* responding in normal mode */
void
ui_resp_norm_mode(Ui* ui, Keyboard_event* key_ev, Ui_resp* resp)
{
  int ch = key_ev->ch;

  /* Do action */
  ui_cat_to_buff(ch, ui);

  /* Set response */
  resp->code = UI_RESP_UPDATE_TEXT;
  resp->buff_txt = ui->buff;
}

/* responding in select mode */
void
ui_resp_sel_mode(Ui* ui, Keyboard_event* key_ev, Ui_resp* resp)
{
  int ch = key_ev->ch;
  int co_ord = 0;
  float amt = 0;
  float small = 0.01f;
  float big = 0.05f;


  /* Do action */
  ui_msg_buff(ui, "Select Size");

  /* Put into function
   * ui_sel_mode_mod(ui, offset)
   * if offset is 0, thats the first corner of sel
   * if offset is 1, thats the other side of sel
   *
   * default         = UI_RESP_UPDATE_TEXT + UI_RESP_SEL_MODE
   * 0 offset sucess = UI_RESP_SEL_FIRST (stay in sel mode, inc offset)
   * 1 offset sucess = UI_RESP_SEL_SECOND - UI_RESP_SEL_MODe (leave sel mode, 0 offset)
   * * offset fail   = UI_RESP_SEL_CANCEL  - UI_RESP_SEL_MODE (leave sel mode, 0 offset)
   *
   * At end send existing resp code plus correct UI_RESP
   *
   * amt_small = 0.01, amt_big = 0.05
   * */

  /* h */
  if(!key_ev->special)
  {
    switch(ch) {
      case 'h':
        co_ord = 0;
        amt =  -big;
        break;
      case 'H':
        co_ord = 0;
        amt =  -small;
        break;
      case 'j':
        co_ord = 1;
        amt =  -big;
        break;
      case 'J':
        co_ord = 1;
        amt =  -small;
        break;
      case 'l':
        co_ord = 0;
        amt =  +big;
        break;
      case 'L':
        co_ord = 0;
        amt =  +small;
        break;
      case 'k':
        co_ord = 1;
        amt =  +big;
        break;
      case 'K':
        co_ord = 1;
        amt =  +small;
        break;
    };
    ui->selection[co_ord] =ui_resize_sel(ui->selection[co_ord], amt);
  }

  /* Set response */
  resp->code = UI_RESP_UPDATE_TEXT + UI_RESP_SEL_MODE;
  resp->selection = ui->selection;
  resp->buff_txt = ui->buff;
}

/* cat char to buffer */
int ui_cat_to_buff(int ch, Ui* ui)
{
  /* Instant clear if the previous buffer write was from a msg */
  if(ui->last_out_msg)
  {
    ui_clear_buff(ui);
    ui->last_out_msg = 0;
  }

  /* If there is space, add the char to the buffer */
  if(ui->buff_len + 1 < MAX_BUFF_LEN)
  {
    size_t dest_len = strlen(ui->buff);
    ui->buff[dest_len++] = ch;
    ui->buff[dest_len] = '\0';
    ui->buff_len++;
    return 1;
  }

  /* Otherwise send a warning to the buffer */
  else
  {
    ui_msg_buff(ui, "Buffer too full!");
    return 0;
  }
}

/* Write null-byte to buffer */
int
ui_clear_buff(Ui* ui){
  char blank[] = "\0";
  ui->buff_len = strlen(blank);
  strcpy(ui->buff, blank);
  return 0;
}


/* Clear the null-byte and the previous letter */
int
ui_backspace_buff(Ui* ui){
  if(ui->last_out_msg) ui_clear_buff(ui);
  else{
    size_t dest_len = strlen(ui->buff);
    ui->buff[dest_len--] = '\0';
    ui->buff[dest_len--] = '\0';
    ui->buff_len = strlen(ui->buff);
  }
}

/* Copy a message to the buffer */
int
ui_msg_buff(Ui* ui, char* msg)
{
  /* Stop printing same msg */
  if(strcmp(ui->buff, msg))  printf("%s\n", msg);

  /* Copy to buffer */
  strcpy(ui->buff, msg);
  ui->buff_len = sizeof(msg);
  ui->last_out_msg = 1;
}

/* could just take the pointer of the position instead
 * and set it if a change is okay*/
float
ui_resize_sel(float cur, float diff)
{
  float new = cur + diff;

  return (new < 1.0f && new > -1.0f) ?
    new:
    cur;
}
