<?php

include "db_connection.php";

if(isset($_GET["code"])) {
    try {
        $query = $db->prepare("INSERT INTO unlockcodes (code) values (?)");
        $params = array($_GET["code"]);
        $query->execute($params);

        $query2 = $db->prepare("UPDATE boeken set safeNumber=1, code=0 WHERE code = ?");
        $params2=array($_GET["code"]);
        $query2->execute($params2);

       if ($query->rowCount()>0) {
            if($query2->rowCount()>0) {
                echo('APIresponseCode gelukt');
            }
            else {
                echo('APIresponseCode mislukt');
            }
        }
        else {
                echo('APIresponseCode mislukt');
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