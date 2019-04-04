<?php

include "db_connection.php";

if(isset($_GET["code"])) {
    try {
        // SQL om de gegevens van het geselecteerde item op te halen
        $query = $db->prepare("DELETE FROM boeken WHERE safeNumber=1");
        $query->execute();

        $query2 = $db->prepare("DELETE FROM unlockcodes WHERE code=?");
        $params2 = array($_GET["code"]);
        $query2-> execute($params2);

        if ($query2->rowCount()>0) {
                if ($query->rowCount()>0) {
                    echo('APIresponseCode gelukt'); //boek is succesvol verwijderd uit unlockcodes & boeken db
                }
                else {
                    echo ('APIresponseCode mislukt'); //boek is succesvol verwijderd uit alleen unlockcodes
                }
        }
        else {
                echo('APIresponseCode mislukt2'); //boek kan niet worden verwijderd
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