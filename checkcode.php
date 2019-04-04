<?php
include "db_connection.php";

// Controle of er een item id via de URL binnen is gekomen en of dit een int is
// Gebruik van filter_var is een handige manier om URL paramaters te checken op geldige waarden
// Door het gebruik van PDO voor de verbinding met de database is het gebruik van filter_var
// niet strikt noodzakelijk meer (PDO heeft standaard beveiliging tegen SQL injection)
// maar voor een goede error handing is het verstandig input variabelen wel te controleren
if(isset($_GET["code"])) {
    try {
        // SQL om de gegevens van het geselecteerde item op te halen
        $query = $db->prepare("SELECT id FROM boeken WHERE code=?");
        $params = array($_GET["code"]);
        $query->execute($params);


        // Haal de rij op (en toon een fout bij problemen)
        // In plaats van FetchAll wordt hier een Fetch gebruikt omdat we zeker weten dat er hooguit 1 rij wordt gevonden
        // Het result is een array met de opgehaalde gegevens (en niet een Array met gegevens van alle opgehaalde rijen zoals bij fetschAll)
        // Omdat er dus maar 1 rij wordt opgehaald hoef je dus ook geen foreach lus te gebruiken
        // De if-constructie wordt gebruikt om te zien of het gevraagde item (met het id dat via de URL binnen komt) wel wordt gevonden (bestaat in de database)
       if ($result = $query->fetch(PDO::FETCH_ASSOC)) {
        $query2 = $db->prepare("SELECT id FROM unlockcodes WHERE code=?");
        $params2 = array($_GET["code"]);
        $query2-> execute($params2);
                if ($result2 = $query2->fetch(PDO::FETCH_ASSOC)) {
                    echo('APIresponseCode 1'); // code geldig om boek te pakken
                }
                else {
                    echo ('APIresponseCode 2'); //boek is alleen geplaatst in webapp
                }
        }
        else {
                echo('APIresponseCode 0');
        }
   }
    catch (PDOException $e) {
        echo "<div class=\"feedbackfout\">Er is een database fout opgetreden. Details: ".$e->getMessage()."</div>";
    }
}
else {
    echo('geen geldige code ingevoerd');
}

?>