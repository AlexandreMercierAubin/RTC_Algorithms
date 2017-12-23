//
// Created by Mario Marchand on 16-12-30.
//

#include "ReseauGTFS.h"
#include <sys/time.h>

using namespace std;

//détermine le temps d'exécution (en microseconde) entre tv2 et tv2
long tempsExecution(const timeval &tv1, const timeval &tv2)
{
    const long unMillion = 1000000;
    long dt_usec = tv2.tv_usec - tv1.tv_usec;
    long dt_sec = tv2.tv_sec - tv1.tv_sec;
    long dtms = unMillion * dt_sec + dt_usec;
    if (dtms < 0) throw logic_error("ReaseauGTFS::tempsExecution(): dtms doit être non négatif");
    return dtms;
}

size_t ReseauGTFS::getNbArcsOrigineVersStations() const
{
    return m_nbArcsOrigineVersStations;
}

size_t ReseauGTFS::getNbArcsStationsVersDestination() const
{
    return m_nbArcsStationsVersDestination;
}

double ReseauGTFS::getDistMaxMarche() const
{
    return distanceMaxMarche;
}

//! \brief construit le réseau GTFS à partir des données GTFS
//! \param[in] Un objet DonneesGTFS
//! \post constuit un réseau GTFS représenté par un graphe orienté pondéré avec poids non négatifs
//! \post initialise la variable m_origine_dest_ajoute à false car les points origine et destination ne font pas parti du graphe
//! \post insère les données requises dans m_arretDuSommet et m_sommetDeArret et construit le graphe m_leGraphe
ReseauGTFS::ReseauGTFS(const DonneesGTFS &p_gtfs)
: m_leGraphe(p_gtfs.getNbArrets()), m_origine_dest_ajoute(false)
{
    //Le graphe possède p_gtfs.getNbArrets() sommets, mais il n'a pas encore d'arcs
    ajouterArcsVoyages(p_gtfs);
    ajouterArcsAttentes(p_gtfs);
    ajouterArcsTransferts(p_gtfs);
}

//! \brief ajout des arcs dus aux voyages
//! \brief insère les arrêts (associés aux sommets) dans m_arretDuSommet et m_sommetDeArret
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsVoyages(const DonneesGTFS & p_gtfs)
{
    try
    {
        //chercher les voyages
        const map <string, Voyage> voyages = p_gtfs.getVoyages();

        //conserver une position dans le graphe du sommet
        long position = 0;
        for (auto itrVoyage = voyages.begin(); itrVoyage != voyages.end(); ++itrVoyage)
        {
            //conserver un pointeur vers le precedent pour faire un lien
            auto itrArretPrecedent = itrVoyage->second.getArrets().begin();
            for (auto itrArret = itrVoyage->second.getArrets().begin();
                 itrArret != itrVoyage->second.getArrets().end(); ++itrArret)
            {
                //ajouter les arrets dans les tableaux de conversion du graphe
                m_arretDuSommet.push_back(*itrArret);
                m_sommetDeArret.insert({*itrArret, position});

                //si l'arret n'est pas le premier
                if (itrArret != itrArretPrecedent)
                {
                    //chercher le sommet des deux arrets
                    auto depart = m_sommetDeArret.find(*itrArretPrecedent);
                    auto arrivee = m_sommetDeArret.find(*itrArret);

                    //calculer le temps
                    int temps = itrArret->get()->getHeureArrivee() - itrArretPrecedent->get()->getHeureDepart();

                    if(temps<0)
                        throw logic_error("arc negatif");

                    //ajouter un arc entre les deux arrets
                    m_leGraphe.ajouterArc(depart->second,
                                          arrivee->second,
                                          temps);

                    //inscrementer le precedent
                    ++itrArretPrecedent;
                }

                ++position;
            }
        }
    }
    catch(...)
    {
        throw logic_error("Une erreur s'est produite dans ajouterArcVoyages");
    }
}

//! \brief ajout des arcs dus aux attentes à chaque station
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsAttentes(const DonneesGTFS & p_gtfs)
{
    try
    {
        //pour toutes les stations
        for (auto itrStation = p_gtfs.getStations().begin(); itrStation != p_gtfs.getStations().end(); ++itrStation)
        {

            auto itrArretPrecedent = itrStation->second.getArrets().begin();
            //pour tous les arrets
            for (auto itrArret = itrStation->second.getArrets().begin();
                 itrArret != itrStation->second.getArrets().end(); ++itrArret)
            {
                //si ce n'est pas le premier et si le voyage n'est pas identique
                if (itrArret != itrArretPrecedent
                    && itrArret->second.get()->getVoyageId()!=itrArretPrecedent->second.get()->getVoyageId())
                {
                    //chercher le sommet des deux arrets
                    auto depart = m_sommetDeArret.find(itrArretPrecedent->second);
                    auto arrivee = m_sommetDeArret.find(itrArret->second);

                    //calculer le temps
                    int temps = itrArret->second.get()->getHeureArrivee() - itrArretPrecedent->second.get()->getHeureArrivee();

                    if(temps<0)
                        throw logic_error("arc negatif");

                    //ajouter un arc entre les deux arrets
                    m_leGraphe.ajouterArc(depart->second,
                                          arrivee->second,
                                          temps);

                    //ajuster le precedent
                    itrArretPrecedent=itrArret;
                }
            }
        }
    }
    catch(...)
    {
        throw logic_error("Une erreur s'est produite dans ajouterArcVoyages");
    }
}

//! \brief ajouts des arcs dus aux transferts entre stations
//! \throws logic_error si une incohérence est détecté lors de cette étape de construction du graphe
void ReseauGTFS::ajouterArcsTransferts(const DonneesGTFS & p_gtfs)
{
    try
    {
        auto vectorTransfert = p_gtfs.getTransferts();

        for (int i = 0; i < vectorTransfert.size(); ++i) // pour tous les transferts
        {
            //chercher les stations
            auto itrStationDepart = p_gtfs.getStations().find(get<0>(vectorTransfert[i]));
            auto itrStationArrivee = p_gtfs.getStations().find(get<1>(vectorTransfert[i]));

            //conserver le temps de transfert
            int tempsTransfert = get<2>(vectorTransfert[i]);

            //pour tous les arrets de la station de depart
            for (auto itrArretDepart = itrStationDepart->second.getArrets().begin();
                 itrArretDepart != itrStationDepart->second.getArrets().end(); ++itrArretDepart)
            {

                //conserver l'heure
                Heure heureDepart = itrArretDepart->second.get()->getHeureArrivee();
                heureDepart = heureDepart.add_secondes(tempsTransfert);

                //cherche la station d'arrivee avec le temps d'attente le plus bas
                auto itrArretArrivee = itrStationArrivee->second.getArrets().lower_bound(heureDepart);

                //si la station existe creer un arc.
                if (itrArretArrivee != itrStationArrivee->second.getArrets().end())
                {
                    auto itrSommetDepart = m_sommetDeArret.find(itrArretDepart->second);
                    auto itrSommetArrivee = m_sommetDeArret.find(itrArretArrivee->second);

                    //calculer le temps
                    int temps = itrArretArrivee->second.get()->getHeureArrivee() - itrArretDepart->second.get()->getHeureArrivee();

                    if(temps<0)
                        throw logic_error("arc negatif");

                    //ajouter l'arc entre les arrets
                    m_leGraphe.ajouterArc(itrSommetDepart->second, itrSommetArrivee->second,temps);
                }
            }
        }
    }
    catch(...)
    {
        throw logic_error("Une erreur s'est produite dans ajouterArcTransfert");
    }
}

//! \brief ajoute des arcs au réseau GTFS à partir des données GTFS
//! \brief Il s'agit des arcs allant du point origine vers une station si celle-ci est accessible à pieds et des arcs allant d'une station vers le point destination
//! \param[in] p_gtfs: un objet DonneesGTFS
//! \param[in] p_pointOrigine: les coordonnées GPS du point origine
//! \param[in] p_pointDestination: les coordonnées GPS du point destination
//! \throws logic_error si une incohérence est détecté lors de la construction du graphe
//! \post constuit un réseau GTFS représenté par un graphe orienté pondéré avec poids non négatifs
//! \post assigne la variable m_origine_dest_ajoute à true (car les points orignine et destination font parti du graphe)
//! \post insère dans m_sommetsVersDestination les numéros de sommets connctés au point destination
void ReseauGTFS::ajouterArcsOrigineDestination(const DonneesGTFS &p_gtfs, const Coordonnees &p_pointOrigine,
                                               const Coordonnees &p_pointDestination)
{
    try
    {
        m_nbArcsOrigineVersStations=0;
        m_nbArcsStationsVersDestination=0;

        Arret::Ptr arretOrigine = make_shared<Arret>(stationIdOrigine, Heure(), Heure(), 0, "origine");
        Arret::Ptr arretDestination = make_shared<Arret>(stationIdDestination, Heure(), Heure(), 1, "destination");

        m_arretDuSommet.push_back(arretOrigine);
        m_sommetOrigine = m_sommetDeArret.size();
        m_sommetDeArret.insert({arretOrigine, m_sommetDeArret.size()});

        m_arretDuSommet.push_back(arretDestination);
        m_sommetDestination = m_sommetDeArret.size();
        m_sommetDeArret.insert({arretDestination, m_sommetDeArret.size()});

        m_leGraphe.resize(m_arretDuSommet.size());


        //ajout des arcs à pieds entre le point source et les arrets des stations atteignables
        Heure tempsDebut = p_gtfs.getTempsDebut();

        //pour toutes les stations
        for (auto itrStation = p_gtfs.getStations().begin(); itrStation != p_gtfs.getStations().end(); ++itrStation)
        {
            //calculer la distance a marcher
            double distanceMarche = abs(itrStation->second.getCoords()-p_pointOrigine);

            //si la station n'est pas trop loin
            if(distanceMarche <= distanceMaxMarche)
            {
                //calculer le temps de marche vers l'arret
                int tempsMarche = round((distanceMarche/vitesseDeMarche)*3600);

                //calculer l'heure d'arrivee
                Heure heureArrivee = tempsDebut;
                heureArrivee=heureArrivee.add_secondes(tempsMarche);

                //trouver l'arret le plus proche en temps
                auto itrArret = itrStation->second.getArrets().lower_bound(heureArrivee);

                //si l'arret existe
                if(itrArret!= itrStation->second.getArrets().end())
                {

                    //trouver son sommet
                    auto itrSommet = m_sommetDeArret.find(itrArret->second);

                    //calculer le temps du deplacement
                    int temps = itrArret->second.get()->getHeureArrivee() - tempsDebut;

                    if(temps<0)
                        throw logic_error("arc negatif");

                    //ajouter l'arc entre l'arret et l'origine
                    m_leGraphe.ajouterArc(m_sommetOrigine, itrSommet->second, temps);
                    ++m_nbArcsOrigineVersStations;
                }
            }

            //ajout des arcs à pieds des arrêts de certaine stations vers l'arret point destination

            //calculer la distance a marcher
            distanceMarche = abs(itrStation->second.getCoords()-p_pointDestination);

            //si la station n'est pas trop loin
            if(distanceMarche<=distanceMaxMarche)
            {
                //calculer le temps de marche vers l'arret
                int tempsMarche = (distanceMarche/vitesseDeMarche)*3600;

                //calculer l'heure d'arrivee
                Heure heureArrivee = tempsDebut;
                heureArrivee=heureArrivee.add_secondes(tempsMarche);

                //pour tous les arrets
                for(auto itrArret = itrStation->second.getArrets().begin(); itrArret != itrStation->second.getArrets().end();++itrArret)
                {
                    auto itrSommet = m_sommetDeArret.find(itrArret->second);

                    m_sommetsVersDestination.push_back(itrSommet->second);

                    //ajouter l'arc entre l'arret et la destination
                    m_leGraphe.ajouterArc(itrSommet->second, m_sommetDestination, tempsMarche);
                    ++m_nbArcsStationsVersDestination;
                }
            }
        }

        m_origine_dest_ajoute=true;
    }
    catch(...)
    {
        throw logic_error("Une erreur s'est produite dans ajouterArcOrigineDestination");
    }
}

//! \brief Remet ReseauGTFS dans l'était qu'il était avant l'exécution de ReseauGTFS::ajouterArcsOrigineDestination()
//! \param[in] p_gtfs: un objet DonneesGTFS
//! \throws logic_error si une incohérence est détecté lors de la modification du graphe
//! \post Enlève de ReaseauGTFS tous les arcs allant du point source vers un arrêt de station et ceux allant d'un arrêt de station vers la destination
//! \post assigne la variable m_origine_dest_ajoute à false (les points orignine et destination sont enlevés du graphe)
//! \post enlève les données de m_sommetsVersDestination
void ReseauGTFS::enleverArcsOrigineDestination()
{
    try
    {
        //enlever les arcs vers la destination
        for(int i=0; i<m_sommetsVersDestination.size();++i)
        {
            m_leGraphe.enleverArc(m_sommetsVersDestination[i],m_sommetDestination);
        }
        m_sommetsVersDestination.clear();

        //enlever les sommets origine et destination
        m_leGraphe.resize(m_leGraphe.getNbSommets()-2);
        for(int i=0; i<2;++i)
        {
            m_sommetDeArret.erase(m_arretDuSommet[m_arretDuSommet.size()-1]);
            m_arretDuSommet.pop_back();
        }

        //retour a la valeur de base
        m_nbArcsOrigineVersStations = 0;
        m_nbArcsStationsVersDestination = 0;
        m_origine_dest_ajoute = false;
    }catch(...)
    {
        throw logic_error("Une erreur s'est produite dans enleverArcsOrigineDestination");
    }
}


//! \brief Trouve le plus court chemin menant du point d'origine au point destination préalablement choisis
//! \brief Permet également d'affichier l'itinéraire du voyage et retourne le temps d'exécution de l'algorithme de plus court chemin utilisé
//! \param[in] p_afficherItineraire: true si on désire afficher l'itinéraire et false autrement
//! \param[out] p_tempsExecution: le temps d'exécution de l'algorithme de plus court chemin utilisé
//! \throws logic_error si un problème survient durant l'exécution de la méthode
void ReseauGTFS::itineraire(const DonneesGTFS &p_gtfs, bool p_afficherItineraire, long &p_tempsExecution) const
{
    if (!m_origine_dest_ajoute)
        throw logic_error(
                "ReseauGTFS::afficherItineraire(): il faut ajouter un point origine et un point destination avant d'obtenir un itinéraire");

    vector<size_t> chemin;

    timeval tv1;
    timeval tv2;
    if (gettimeofday(&tv1, 0) != 0)
        throw logic_error("ReseauGTFS::afficherItineraire(): gettimeofday() a échoué pour tv1");
    unsigned int tempsDuTrajet = m_leGraphe.plusCourtChemin(m_sommetOrigine, m_sommetDestination, chemin);
    if (gettimeofday(&tv2, 0) != 0)
        throw logic_error("ReseauGTFS::afficherItineraire(): gettimeofday() a échoué pour tv2");
    p_tempsExecution = tempsExecution(tv1, tv2);

    if (tempsDuTrajet == numeric_limits<unsigned int>::max())
    {
        if (p_afficherItineraire)
            cout << "La destination n'est pas atteignable de l'orignine durant cet intervalle de temps" << endl;
        return;
    }

    if (tempsDuTrajet == 0)
    {
        if (p_afficherItineraire) cout << "Vous êtes déjà situé à la destination demandée" << endl;
        return;
    }

    //un chemin non trivial a été trouvé
    if (chemin.size() <= 2)
        throw logic_error("ReseauGTFS::afficherItineraire(): un chemin non trivial doit contenir au moins 3 sommets");
    if (m_arretDuSommet[chemin[0]]->getStationId() != stationIdOrigine)
        throw logic_error("ReseauGTFS::afficherItineraire(): le premier noeud du chemin doit être le point origine");
    if (m_arretDuSommet[chemin[chemin.size() - 1]]->getStationId() != stationIdDestination)
        throw logic_error(
                "ReseauGTFS::afficherItineraire(): le dernier noeud du chemin doit être le point destination");

    if (p_afficherItineraire)
    {
        std::cout << std::endl;
        std::cout << "=====================" << std::endl;
        std::cout << "     ITINÉRAIRE      " << std::endl;
        std::cout << "=====================" << std::endl;
        std::cout << std::endl;
    }

    if (p_afficherItineraire) cout << "Heure de départ du point d'origine: "  << p_gtfs.getTempsDebut() << endl;
    Arret::Ptr ptr_a = m_arretDuSommet.at(chemin[0]);
    Arret::Ptr ptr_b = m_arretDuSommet.at(chemin[1]);
    if (p_afficherItineraire)
        cout << "Rendez vous à la station " << p_gtfs.getStations().at(ptr_b->getStationId()) << endl;

    unsigned int sommet = 1;

    while (sommet < chemin.size() - 1)
    {
        ptr_a = ptr_b;
        ++sommet;
        ptr_b = m_arretDuSommet.at(chemin[sommet]);
        while (ptr_b->getStationId() == ptr_a->getStationId())
        {
            ptr_a = ptr_b;
            ++sommet;
            ptr_b = m_arretDuSommet.at(chemin[sommet]);
        }
        //on a changé de station
        if (ptr_b->getStationId() == stationIdDestination) //cas où on est arrivé à la destination
        {
            if (sommet != chemin.size() - 1)
                throw logic_error(
                        "ReseauGTFS::afficherItineraire(): incohérence de fin de chemin lors d'un changement de station");
            break;
        }
        if (sommet == chemin.size() - 1)
            throw logic_error("ReseauGTFS::afficherItineraire(): on ne devrait pas être arrivé à destination");
        //on a changé de station mais sommet n'est pas le noeud destination
        string voyage_id_a = ptr_a->getVoyageId();
        string voyage_id_b = ptr_b->getVoyageId();
        if (voyage_id_a != voyage_id_b) //on a changé de station à pieds
        {
            if (p_afficherItineraire)
                cout << "De cette station, rendez-vous à pieds à la station " << p_gtfs.getStations().at(ptr_b->getStationId()) << endl;
        }
        else //on a changé de station avec un voyage
        {
            Heure heure = ptr_a->getHeureArrivee();
            unsigned int ligne_id = p_gtfs.getVoyages().at(voyage_id_a).getLigne();
            string ligne_numero = p_gtfs.getLignes().at(ligne_id).getNumero();
            if (p_afficherItineraire)
                cout << "De cette station, prenez l'autobus numéro " << ligne_numero << " à l'heure " << heure << " "
                     << p_gtfs.getVoyages().at(voyage_id_a) << endl;
            //maintenant allons à la dernière station de ce voyage
            ptr_a = ptr_b;
            ++sommet;
            ptr_b = m_arretDuSommet.at(chemin[sommet]);
            while (ptr_b->getVoyageId() == ptr_a->getVoyageId())
            {
                ptr_a = ptr_b;
                ++sommet;
                ptr_b = m_arretDuSommet.at(chemin[sommet]);
            }
            //on a changé de voyage
            if (p_afficherItineraire)
                cout << "et arrêtez-vous à la station " << p_gtfs.getStations().at(ptr_a->getStationId()) << " à l'heure "
                     << ptr_a->getHeureArrivee() << endl;
            if (ptr_b->getStationId() == stationIdDestination) //cas où on est arrivé à la destination
            {
                if (sommet != chemin.size() - 1)
                    throw logic_error(
                            "ReseauGTFS::afficherItineraire(): incohérence de fin de chemin lors d'u changement de voyage");
                break;
            }
            if (ptr_a->getStationId() != ptr_b->getStationId()) //alors on s'est rendu à pieds à l'autre station
                if (p_afficherItineraire)
                    cout << "De cette station, rendez-vous à pieds à la station " << p_gtfs.getStations().at(ptr_b->getStationId()) << endl;
        }
    }

    if (p_afficherItineraire)
    {
        cout << "Déplacez-vous à pieds de cette station au point destination" << endl;
        cout << "Heure d'arrivée à la destination: " << p_gtfs.getTempsDebut().add_secondes(tempsDuTrajet) << endl;
    }
    unsigned int h = tempsDuTrajet / 3600;
    unsigned int reste_sec = tempsDuTrajet % 3600;
    unsigned int m = reste_sec / 60;
    unsigned int s = reste_sec % 60;
    if (p_afficherItineraire)
    {
        cout << "Durée du trajet: " << h << " heures, " << m << " minutes, " << s << " secondes" << endl;
    }

}


