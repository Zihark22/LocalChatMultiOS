#-------------
#   Chat Client/Serveur
#-------------
# Compiling client+server and executing the server
# Command : ./compileAll.sh

clear && printf '\e[3J' ;

compile=false;


g++ -o serveur_Mac serveur.cpp  # compilation serveur
if [ $? -eq 0 ]; then
  echo "\033[36mCompilation server sucessful \033[0m"
  g++ -o client_Mac client.cpp  # compilation client
  if [ $? -eq 0 ]; then
    echo "\033[36mCompilation client sucessful \033[0m"
  else
    echo "\033[31mCompilation client failed !!\033[0m"
  fi
else
  echo "\033[31mCompilation server failed !!\033[0m"
fi