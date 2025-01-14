# Telemetry
Proiectul este alcatuit dintr-un daemon si o librarie care expune functionalitatea daemonului.

Daemonul se ocupa cu distributia de mesaje pe canale de distributie, intre mai multi participanti.

Canalele de distributie pot fi organizate ierarhic, in asa fel incat un mesaj trimis pe un canal 'parinte' va fi trimis si pe canalele 'copil'.

# Distribuire task-uri:
* **Pascaru Dan Alexandru** – Reprezentare canale
* **Petre Robert Cristian** – Comunicare librarie-daemon
* **Popescu Tiberiu** – Comunicare user-canale
* **Voicu Ioan Vladut** – Creare daemon

# Daemonizare
Daemonul respecta modelul de "new-style daemons". Daemonizarea este realizata printr-o librarie C header-only, aceasta permitand daemonizarea in asa fel incat sa se creeze un PID file, sa ofere optiunea de logging (expunand o interfata dedicata) si sa urmeze desigur pasii necesari daemonizarii conforme.

Libraria daemon.h expune un API de daeonizare cu urmatoarele functionalitati:
- pornire si event-loop pentru un daemon;
- API secundar de logging
- integrare systemd (scuze, dar acapareaza lumea)

Pentru a folosi daemon.h:

- creati un `daemon_config_t` cu setarile dorite: nivelul de logging, working dir, umask, etc., logica aplicatiei fiind un callback pentru care puteti furniza argumente, tipic UNIX;
- apelati, pasand structul creat anterior, functia initialize_daemon(); eventual, daca este cazul, creati un handler pentru a procesa perechi KV, si folositi functia parse_config_file();
- este recomandat daca ati folosit functionalitatea pentru configurari sa creati un handler care sa reincarce configuratia, si setati-l pentru a fi apelat in cazul SIGHUP cu functia register_config_handler;
- apelati daemon_run();
- daca intentionati sa integrati cu systemd, aveti la dispozitie daemon_notify_status();
- de preferat, in codul dumneavoastra, verificati starea daemonului si actionati corespunzator:
  - daemon_should_reload() returneaza daca s-a primit SIGHUP; in caz afirmativ, daca s-a setat deja handlerul de reload, va rog pregatiti daemonul pentru a redeschide socket-uri, fisiere, etc.;
  - daemon_is_running() returneaza daca inca ruleaza daemonul; in caz in care nu mai ruleaza, inchideti inainte sa se termine event-loop-ul toate socket-urile si descriptorii de fisiere deschis;

Exemplu de aplicatie:
```
#include "daemon.h"
#include <unistd.h>
#include <stdio.h>

// Application logic callback
void app_logic_callback(void *user_data) {
    static int counter = 0;
    log_info("App logic running, counter: %d", counter);
    
    // Simulate some work
    sleep(1);
    
    counter++;
}

int main(int argc, char *argv[]) {
    // Initialize the daemon configuration
    daemon_config_t config = {
        .log_level = LOG_LEVEL_DEBUG,      // Set log level to DEBUG for detailed logs
        .working_directory = "/tmp",       // Set the working directory (can be any valid path)
        .umask_value = 027,                // Set a restrictive umask
        .config_file = "/tmp/daemon.conf", // A placeholder path for the config file
        .run_callback = app_logic_callback, // Your application logic callback
        .user_data = NULL                  // No additional user data for this simple test
    };

    // Initialize the daemon
    if (initialize_daemon(&config) != 0) {
        log_error("Failed to initialize daemon");
        return 1;
    }

    // Run the daemon
    log_info("Daemon is running...");
    return daemon_run(&config);
}

```

# Reprezentare canale

## Libraria dir.h
### Crearea si stergerea directoarelor
- **create_dir**: Permite crearea de directoare multiple(separate prin '/') si configurarea initiala a fisierelor de log(log.txt) in fiecare director nou creat.
- **remove_dir**: Permite stergerea directoarelor si a intregului continut din ele si din adancimea lor.

### Gestionarea fisierelor de log
- **open_log**: Deschide un fisier de log dintr-o ruta specificata si il returneaza.

### Parcurgerea directoarelor
- **get_dir**: Deschide si returneaza toate subdirectoarele dintr-un director sub forma unei liste simplu inlantuite sub forma de ruta absoluta.
- **Structura listei**: list->path, list->next si mereu se termina cu NULL.

# Comunicare librarie-daemon
## Libraria telemetry.h
Utilizata pentru a comunica cu daemon-ul.
### Crearea si stergerea canalelor de comunicatie
- **tlm_open**: Permite crearea de conexiuni cu un daemon la adresa si portul specificate, avand scopul de a comunica cu canalul dorit.
- **tlm_close**: Inchide conexiunea. Tokenul primit de la **tlm_open** nu mai este valabil.
- **tlm_type**: Iti permite sa vizualizezi tipul de conexiune al unui token. Poate fi: **TLM_PUBLISHER**,**TLM_SUBSCRIBER**,**TLM_BOTH** sau **TLM_CLOSED**.
### Transmitere mesaje
- **tlm_post**: Trimite un mesaj catre un canal, fiind necesar tokenul obtinut cu **tlm_open**.
### Citire mesaje
- **tlm_read**: Citeste un mesaj de pe un canal, fiind necesar tokenul obtinut cu **tlm_open**.

# Potentiale imbunatatiri
* Encriptare mesaje
* Adaugarea de UID pentru a distinge intre mesajele utilizatorilor
* Comunicarea cu serverul printr-un pseudoterminal
* Server concurent (in prezent este iterativ)

