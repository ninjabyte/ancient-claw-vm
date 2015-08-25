#define __WIN32__ 1

#include <SFML/Graphics.h>

int main() {
  sfVideoMode mode = {800, 600, 32};
  sfRenderWindow* window;

  window = sfRenderWindow_create(mode, "SFML Window", sfResize | sfClose, NULL);

  if(!window)
    return -1;

  while(sfRenderWindow_isOpen(window)) {

  }

  return 0;
}
