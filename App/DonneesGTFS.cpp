//
// Created by Mario Marchand on 16-12-29.
//

#include "DonneesGTFS.h"

using namespace std;



//! \brief construit un objet GTFS
//! \param[in] p_date: la date utilisée par le GTFS
//! \param[in] p_now1: l'heure du début de l'intervalle considéré
//! \param[in] p_now2: l'heure de fin de l'intervalle considéré
//! \brief Ces deux heures définissent l'intervalle de temps du GTFS; seuls les moments de [p_now1, p_now2) sont considérés
DonneesGTFS::DonneesGTFS(const Date &p_date, const Heure &p_now1, const Heure &p_now2)
        : m_date(p_date), m_now1(p_now1), m_now2(p_now2), m_nbArrets(0), m_tousLesArretsPresents(false)
{
}

//! \brief partitionne un string en un vecteur de strings
//! \param[in] s: le string à être partitionner
//! \param[in] delim: le caractère utilisé pour le partitionnement
//! \return le vecteur de string sans le caractère utilisé pour le partitionnement
vector<string> DonneesGTFS::string_to_vector(const string &s, char delim)
{
    stringstream ss(s);
    string item;
    vector<string> elems;
    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}


//! \brief ajoute les lignes dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les lignes
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterLignes(const std::string &p_nomFichier)
{
    //tenter l'ouverture du fichier
    std::ifstream ifFichier(p_nomFichier);
    if(ifFichier.is_open())
    {
        try
        {
            //sauter la ligne 1
            std::string strLigne;
            std::getline(ifFichier, strLigne);

            //boucler les lignes
            while (std::getline(ifFichier, strLigne))
            {
                //enlever les guillemets
                strLigne.erase(std::remove(strLigne.begin(), strLigne.end(), '"'), strLigne.end());
                std::vector<std::string> vObjet = string_to_vector(strLigne, ',');

                //convertir vers unsigned int p_id, const std::string & p_numero, const std::string & p_description, const CategorieBus&

                //insérer les données dans m_lignes
                int id = std::stoul(vObjet[0]);
                m_lignes.insert({id, Ligne(id, vObjet[2], vObjet[4], Ligne::couleurToCategorie(vObjet[7]))});
                m_lignes_par_numero.insert(
                        {vObjet[2], Ligne(id, vObjet[2], vObjet[4], Ligne::couleurToCategorie(vObjet[7]))});


            }
            ifFichier.close();
        }
        catch(...)//attraper une erreur si elle survient
        {
            throw std::logic_error("erreur logique");
        }
    }
    else
    {
        throw std::logic_error("le fichier n'a pas pu ouvrir");
    }
}

//! \brief ajoute les stations dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterStations(const std::string &p_nomFichier)
{
    //tenter l'ouverture du fichier
    std::ifstream ifFichier(p_nomFichier);

    if(ifFichier.is_open())
    {
        try
        {
            //sauter la ligne 1
            std::string strLigne;
            std::getline(ifFichier, strLigne);

            //boucler les lignes
            while (std::getline(ifFichier, strLigne)) {
                //enlever les guillemets
                strLigne.erase(std::remove(strLigne.begin(), strLigne.end(), '"'), strLigne.end());
                std::vector<std::string> vObjet = string_to_vector(strLigne, ',');

                //convertir vers unsigned int p_id, const std::string & p_nom, const std::string & p_description,const Coordonnees & p_coords

                //insérer les données dans m_stations
                int id = std::stoul(vObjet[0]);
                Coordonnees location(std::stod(vObjet[3]), std::stod(vObjet[4]));
                m_stations.insert({id, Station(id, vObjet[1], vObjet[2], location)});


            }
            ifFichier.close();
        }
        catch(...)//attraper une erreur si elle survient
        {
            throw std::logic_error("erreur logique");
        }
    }
    else
    {
        throw std::logic_error("le fichier n'a pas pu ouvrir");
    }
}

//! \brief ajoute les transferts dans l'objet GTFS
//! \breif Cette méthode doit âtre utilisée uniquement après que tous les arrêts ont été ajoutés
//! \brief les transferts (entre stations) ajoutés sont uniquement ceux pour lesquelles les stations sont prensentes dans l'objet GTFS
//! \brief les transferts d'une station à elle m^e ne sont pas ajoutés
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
//! \throws logic_error si tous les arrets de la date et de l'intervalle n'ont pas été ajoutés
void DonneesGTFS::ajouterTransferts(const std::string &p_nomFichier) {


    if(m_tousLesArretsPresents) // vérifier que tous les arrêts sont présents
    {
        //tenter l'ouverture du fichier
        std::ifstream ifFichier(p_nomFichier);
        if (ifFichier.is_open())
        {

            try
            {

                //sauter la ligne 1
                std::string strLigne;
                std::getline(ifFichier, strLigne);

                //boucler les lignes
                while (std::getline(ifFichier, strLigne))
                {


                    //enlever les guillemets
                    strLigne.erase(std::remove(strLigne.begin(), strLigne.end(), '"'), strLigne.end());
                    std::vector<std::string> vObjet = string_to_vector(strLigne, ',');

                    //convertir vers unsigned int

                    unsigned int uiFrom = std::stoul(vObjet[0]);
                    unsigned int uiTo = std::stoul(vObjet[1]);

                    if (uiFrom != uiTo) // empêcher le transfert d'une station a elle-meme
                    {

                        unsigned int uiTime = std::stoul(vObjet[3]);

                        if (uiTime == 0)//si le temps est 0, assigner a 1 seconde
                            uiTime = 1;

                        //si les stations existent
                        std::map<unsigned int, Station>::iterator itrFrom = m_stations.find(uiFrom);
                        std::map<unsigned int, Station>::iterator itrTo = m_stations.find(uiTo);

                        //si la station de départ et la station d'arrivée existent
                        if (itrFrom != m_stations.end() && itrTo != m_stations.end())
                        {
                            //insérer les données dans m_transferts
                            m_transferts.push_back(
                                    std::tuple<unsigned int, unsigned int, unsigned int>(
                                            uiFrom,
                                            uiTo,
                                            uiTime
                                    )
                            );
                        }
                    }


                }
                ifFichier.close();
            }
            catch (...)//attraper une erreur si elle survient
            {
                throw std::logic_error("erreur logique ajouterArretsDesVoyagesDeLaDate");
            }
        }
        else
        {
            throw std::logic_error("le fichier n'a pas pu ouvrir");
        }
    }
    else
    {
        //Il faut rouler AjouterArretDesVoyages avant.
        throw std::logic_error("Il faut que tous les arrêts soient présents");
    }
}


//! \brief ajoute les services de la date du GTFS (m_date)
//! \param[in] p_nomFichier: le nom du fichier contenant les services
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterServices(const std::string &p_nomFichier)
{

    //tenter l'ouverture du fichier
    std::ifstream ifFichier(p_nomFichier);
    if(ifFichier.is_open())
    {
        try
        {
            //sauter la ligne 1
            std::string strLigne;
            std::getline(ifFichier, strLigne);

            //boucler les lignes
            while (std::getline(ifFichier, strLigne))
            {

                //enlever les guillemets
                strLigne.erase(std::remove(strLigne.begin(), strLigne.end(), '"'), strLigne.end());
                std::vector<std::string> vObjet = string_to_vector(strLigne, ',');

                //convertir vers unsigned int p_id, const std::string & p_nom, const std::string & p_description,const Coordonnees & p_coords

                Date dateBus(std::stoul(vObjet[1].substr(0, 4)), std::stoul(vObjet[1].substr(4, 2)),
                             std::stoul(vObjet[1].substr(6, 2)));
                int exception = std::stoul(vObjet[2]);

                //si exception est de type 1 et le service est actif à la date fournie
                if (exception == 1 && dateBus == m_date)
                {
                    //insérer l'id du service dans m_services
                    m_services.insert(vObjet[0]);
                }


            }
            ifFichier.close();
        }
        catch(...)//attraper une erreur si elle survient
        {
            throw std::logic_error("erreur logique");
        }
    }
    else
    {
        throw std::logic_error("le fichier n'a pas pu ouvrir");
    }
}

//! \brief ajoute les voyages de la date
//! \brief seuls les voyages dont le service est présent dans l'objet GTFS sont ajoutés
//! \param[in] p_nomFichier: le nom du fichier contenant les voyages
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterVoyagesDeLaDate(const std::string &p_nomFichier)
{
    //tenter l'ouverture du fichier
    std::ifstream ifFichier(p_nomFichier);
    if(ifFichier.is_open())
    {
        try
        {
            //sauter la ligne 1
            std::string strLigne;
            std::getline(ifFichier, strLigne);

            //boucler les lignes
            while (std::getline(ifFichier, strLigne))
            {

                //enlever les guillemets
                strLigne.erase(std::remove(strLigne.begin(), strLigne.end(), '"'), strLigne.end());
                std::vector<std::string> vObjet = string_to_vector(strLigne, ',');

                //convertir vers const std::string & p_id, unsigned int p_ligne_id, const std::string & p_service_id, const std::string & p_destination

                //les services existent a la m_date, donc si le voyage a un service qui existe,il est a la m_date
                std::unordered_set<std::string>::iterator itrService = m_services.find(vObjet[1]);
                if (itrService != m_services.end())// si le service existe (implique que le voyage est à m_date)
                {
                    //insérer les données dans m_voyages
                    m_voyages.insert({vObjet[2], Voyage(vObjet[2], std::stoul(vObjet[0]), vObjet[1], vObjet[3])});
                }


            }
            ifFichier.close();
        }
        catch(...)//attraper une erreur si elle survient
        {
            throw std::logic_error("erreur logique");
        }
    }
    else
    {
        throw std::logic_error("le fichier n'a pas pu ouvrir");
    }
}

//! \brief ajoute les arrets aux voyages présents dans le GTFS si l'heure du voyage appartient à l'intervalle de temps du GTFS
//! \brief De plus, on enlève les voyages qui n'ont pas d'arrêts dans l'intervalle de temps du GTFS
//! \brief De plus, on enlève les stations qui n'ont pas d'arrets dans l'intervalle de temps du GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les arrets
//! \post assigne m_tousLesArretsPresents à true
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterArretsDesVoyagesDeLaDate(const std::string &p_nomFichier)
{
    //tenter l'ouverture du fichier
    std::ifstream ifFichier(p_nomFichier);
    if(ifFichier.is_open())
    {
        try
        {
            //sauter la ligne 1
            std::string strLigne;
            std::getline(ifFichier, strLigne);

            //boucler les lignes

            while (std::getline(ifFichier, strLigne))
            {
                //enlever les guillemets
                strLigne.erase(std::remove(strLigne.begin(), strLigne.end(), '"'), strLigne.end());
                std::vector<std::string> vObjet = string_to_vector(strLigne, ',');

                //convertir vers unsigned int p_station_id, const Heure & p_heure_arrivee, const Heure & p_heure_depart,unsigned int p_numero_sequence, const std::string & p_voyage_id

                //convertion de la partie heure du vecteur en heures
                Heure heureArrive({
                                          (unsigned int) std::stoul(vObjet[1].substr(0, 2)),
                                          (unsigned int) std::stoul(vObjet[1].substr(3, 2)),
                                          (unsigned int) std::stoul(vObjet[1].substr(6, 2))
                                  });
                Heure heureDepart({
                                          (unsigned int) std::stoul(vObjet[2].substr(0, 2)),
                                          (unsigned int) std::stoul(vObjet[2].substr(3, 2)),
                                          (unsigned int) std::stoul(vObjet[2].substr(6, 2))
                                  });

                //si la bus passe avant que la personne soit partie et si la but part après l'arrivée
                if (heureDepart >= m_now1 && heureArrive < m_now2 &&
                    m_voyages.find(vObjet[0]) != m_voyages.end())//si service_id existe
                {
                    //Ajouter les arrets aux voyages
                    Arret::Ptr a_ptr = make_shared<Arret>(std::stoul(vObjet[3]), heureArrive, heureDepart,
                                                          std::stoul(vObjet[4]),
                                                          vObjet[0]);
                    m_voyages[vObjet[0]].ajouterArret(a_ptr);

                    //ajouter les arrets aux stations
                    std::map<unsigned int, Station>::iterator itrStation = m_stations.find(std::stoul(vObjet[3]));
                    if (m_stations.end() != itrStation)
                    {
                        itrStation->second.addArret(a_ptr);
                    }
                    m_nbArrets++;
                }

            }

            //boucle les voyages pour enlever les voyages sans arrets
            std::map<std::string, Voyage>::iterator itrVoyage = m_voyages.begin();
            while (itrVoyage != m_voyages.end())
            {
                unsigned int uiArrets = itrVoyage->second.getNbArrets();

                if (uiArrets > 0)// voyage ayant un arrêt; passer au suivant
                {
                    itrVoyage++;
                }
                else // voyage n'ayant pas d'arrêt; effacer
                {

                    itrVoyage = m_voyages.erase(itrVoyage);
                }
            }

            //boucle dans les stations pour effacer les stations sans arrêt
            std::map<unsigned int, Station>::iterator itrStation = m_stations.begin();
            while (itrStation != m_stations.end())
            {
                if (itrStation->second.getNbArrets() == 0)// station sans arrêt; effacer
                {
                    itrStation = m_stations.erase(itrStation);
                }
                else // station avec arrêt; passer au suivant
                {
                    ++itrStation;
                }
            }
            // tous les arrêts sont présents maintenant!
            m_tousLesArretsPresents = true;

            ifFichier.close();
        }
        catch (...) //attraper une erreur si elle survient
        {
            throw std::logic_error("erreur logique ajouterArretsDesVoyagesDeLaDate");
        }
    }
    else
    {
        throw std::logic_error("le fichier n'a pas pu ouvrir");
    }
}

unsigned int DonneesGTFS::getNbArrets() const
{
    return m_nbArrets;
}

size_t DonneesGTFS::getNbLignes() const
{
    return m_lignes.size();
}

size_t DonneesGTFS::getNbStations() const
{
    return m_stations.size();
}

size_t DonneesGTFS::getNbTransferts() const
{
    return m_transferts.size();
}

size_t DonneesGTFS::getNbServices() const
{
    return m_services.size();
}

size_t DonneesGTFS::getNbVoyages() const
{
    return m_voyages.size();
}

void DonneesGTFS::afficherLignes() const
{
    std::cout << "======================" << std::endl;
    std::cout << "   LIGNES GTFS   " << std::endl;
    std::cout << "   COMPTE = " << m_lignes.size() << "   " << std::endl;
    std::cout << "======================" << std::endl;
    for (const auto & ligneM : m_lignes_par_numero)
    {
        cout << ligneM.second;
    }
    std::cout << std::endl;
}

void DonneesGTFS::afficherStations() const
{
    std::cout << "========================" << std::endl;
    std::cout << "   STATIONS GTFS   " << std::endl;
    std::cout << "   COMPTE = " << m_stations.size() << "   " << std::endl;
    std::cout << "========================" << std::endl;
    for (const auto & stationM : m_stations)
    {
        std::cout << stationM.second << endl;
    }
    std::cout << std::endl;
}

void DonneesGTFS::afficherTransferts() const
{
    std::cout << "========================" << std::endl;
    std::cout << "   TRANSFERTS GTFS   " << std::endl;
    std::cout << "   COMPTE = " << m_transferts.size() << "   " << std::endl;
    std::cout << "========================" << std::endl;
    for (unsigned int i = 0; i < m_transferts.size(); ++i)
    {
        std::cout << "De la station " << get<0>(m_transferts.at(i)) << " vers la station " << get<1>(m_transferts.at(i))
        <<
        " en " << get<2>(m_transferts.at(i)) << " secondes" << endl;
        
    }
    std::cout << std::endl;
    
}


void DonneesGTFS::afficherArretsParVoyages() const
{
    std::cout << "=====================================" << std::endl;
    std::cout << "   VOYAGES DE LA JOURNÉE DU " << m_date << std::endl;
    std::cout << "   " << m_now1 << " - " << m_now2 << std::endl;
    std::cout << "   COMPTE = " << m_voyages.size() << "   " << std::endl;
    std::cout << "=====================================" << std::endl;
    
    for (const auto & voyageM : m_voyages)
    {
        unsigned int ligne_id = voyageM.second.getLigne();
        auto l_itr = m_lignes.find(ligne_id);
        cout << (l_itr->second).getNumero() << " ";
        cout << voyageM.second << endl;
        for (const auto & a: voyageM.second.getArrets())
        {
            unsigned int station_id = a->getStationId();
            auto s_itr = m_stations.find(station_id);
            std::cout << a->getHeureArrivee() << " station " << s_itr->second << endl;
        }
    }
    
    std::cout << std::endl;
}

void DonneesGTFS::afficherArretsParStations() const
{
    std::cout << "========================" << std::endl;
    std::cout << "   ARRETS PAR STATIONS   " << std::endl;
    std::cout << "   Nombre d'arrêts = " << m_nbArrets << std::endl;
    std::cout << "========================" << std::endl;
    for ( const auto & stationM : m_stations)
    {
        std::cout << "Station " << stationM.second << endl;
        for ( const auto & arretM : stationM.second.getArrets())
        {
            string voyage_id = arretM.second->getVoyageId();
            auto v_itr = m_voyages.find(voyage_id);
            unsigned int ligne_id = (v_itr->second).getLigne();
            auto l_itr = m_lignes.find(ligne_id);
            std::cout << arretM.first << " - " << (l_itr->second).getNumero() << " " << v_itr->second << std::endl;
        }
    }
    std::cout << std::endl;
}

const std::map<std::string, Voyage> &DonneesGTFS::getVoyages() const
{
    return m_voyages;
}

const std::map<unsigned int, Station> &DonneesGTFS::getStations() const
{
    return m_stations;
}

const std::vector<std::tuple<unsigned int, unsigned int, unsigned int> > &DonneesGTFS::getTransferts() const
{
    return m_transferts;
}

Heure DonneesGTFS::getTempsFin() const
{
    return m_now2;
}

Heure DonneesGTFS::getTempsDebut() const
{
    return m_now1;
}

const std::unordered_map<unsigned int, Ligne> &DonneesGTFS::getLignes() const
{
    return m_lignes;
}



