<?php

  /**
   * You might have noticed, that the module.inc file for the setup module
   * does not contain any nodes. This means, that you can not grant anyone
   * any rights to use the setup node. 
   * This ensures, that only the administrator user can make use of the 
   * stuff presented here
   */
   
  class setup extends atkNode
  {
    function setup()
    {
      $this->atkNode("setup");
    }
    
    function action_intro()
    {
      $this->setupScreen("Introduction", 
                          "I am the Achievo setup script. I can do several things for you:
                          <ul>
                            <li>I can check for new versions of Achievo or one of your installed modules. 
                            <li>I can install an Achievo database if you don't already have one.
                            <li>I can upgrade an existing Achievo database if necessary.
                            <li>I can do a sanity check on the Achievo database (not yet implemented).                            
                          </ul>
                          <br>I will try to do my job with as little input as possible. I will make
                          educated guesses about what I need to do, based on your current database
                          (if any), and the Achievo config.inc.php file. 
                          <br>If you haven't done so, please make sure your config.inc.php is 
                          correct, before clicking the 'continue' button. In config.inc.php, you
                          have to specify the database I should use and a valid username and password.
                          ",
                          "dbcheck");
    }
    
    function action_dbcheck()
    {            
      $res = $this->getDbStatus();
      
      if ($res==DB_SUCCESS)
      {
        // Database seems to be ok. Now we go check what version(s) we are running.
        return $this->action_versioncheck();
      }
      else
      {
        // something went wrong.      
        return $this->handleDbStatus($res); 
      }          
    }
    
    function getDbStatus()
    {
      global $g_db;
      
      // We don't want the db class to display error messages, because
      // we handle the error ourselves.
      $g_db->m_haltonerror = false;            
      
      $res = $g_db->connect();
      
      if ($res==DB_SUCCESS)
      {        
        // This can't be trusted. Mysql returns DB_SUCCESS even
        // if the user doesn't have access to the database. We only get an
        // error for this after we performed the first query.
        $tmp = $g_db->getrows("show tables");  // TODO FIXME: 'show tables' is NOT database independent
        $res = $g_db->_translateError($g_db->m_errno);
      }
          
      return $res;
    }
    
    /** 
     * Handles the database status. Because of the way some databases
     * handle connections, DB_SUCCESS needs to be confirmed by 
     * performing a query. If handleDbStatus is not called with
     * confirmed=true, it will perform this query and confirm itself.
     */
    function handleDbStatus($dbstatus)
    {    
      switch($dbstatus)
      {
        case DB_SUCCESS: // We're not supposed to handle a db status if the db is ok. But, in the case some 
                         // programmer decides to call handleDbStatus anyway, we'll just return a screen with an 'ok' message.
          return $this->setupScreen("Connection succesful",
                                    "Your database connection appears to be OK");
          
          // In the case we don't have a DB_SUCCESS now, we fall through
          // the other cases (we don't break), so we can handle the error.
        case DB_UNKNOWNHOST:
          return $this->setupScreen("Unable to connect to database-server", 
                                    "In config.inc.php, you have set \$config_databasehost to '<b>".atkconfig("databasehost")."</b>'.
                                     However, I am not able to connect to that server. 
                                     This could mean:
                                     <ul>
                                       <li>The hostname is incorrect. In this case, please correct the error in the config.inc.php file and click 'Retry'.
                                       <li>The host is down. You might want to retry later or contact the servers' administrator.
                                       <li>The host is up, but there's no databaseserver running. You might want to select a different server, or contact your system-administrator.                                       
                                     </ul>", "dbcheck");
          
        case DB_UNKNOWNDATABASE:
          return $this->setupScreen("Database does not exist", "database doesn't exist");
          
        case DB_ACCESSDENIED_USER:
          return $this->setupScreen("Database username or password incorrect", 
                                    "I've tried to connect to the database with username '".atkconfig("databaseuser")."' and the password you specified in config.inc.php, but was
                                     unable to login. 
                                     <br>This could mean:
                                     <ul>
                                       <li>The username and or password in config.inc.php are not correct. In this case, please correct the error and click 'Retry'.
                                       <li>The user has not been created yet in the databaseserver. In this case, create the useraccount or contact your system-administrator. When the user is created, click 'Retry'.".
                                       (atkconfig("database")!="mysql"?"":"<li>A common problem when using MySQL is that the user was created, but the privileges were not flushed. You might want to try reloading the database (or issuing a 'FLUSH PRIVILEGES' statement), and clicking 'Retry'.").
                                    "</ul>",
                                    "dbcheck");
          
        case DB_ACCESSDENIED_DB:
          return $this->setupScreen("User not allowed to connect to database", 
                                    "I've tried to connect to the database with username '".atkconfig("databaseuser")."' and the password you specified in config.inc.php, but was 
                                     unable to connect. The username and password appear to be ok, but the user is not allowed to use the database '".atkconfig("databasename")."'.
                                     <br>This could mean:
                                     <ul>
                                       <li>The username has not been granted access to the database. Please correct the error and click 'Retry'".
                                       (atkconfig("database")!="mysql"?"":"<li>A common problem when using MySQL is that access was granted, but the privileges were not flushed. You might want to try reloading the database (or issuing a 'FLUSH PRIVILEGES' statement), and clicking 'Retry'.").
                                    "</ul>",
                                    "dbcheck");
              
        default: // catches any errors we don't know of, and also DB_UNKNOWNERROR
          return $this->setupScreen("Unknown connection problem",
                                    "There was an unknown problem when I tried connecting to the database. Please verify your database setup and
                                     the database configuration in config.inc.php. If you think you have fixed the problem, you may click 'Retry'.",
                                    "dbcheck");
      }
    }
    
    function setupScreen($title, $output, $nextaction="")
    {
      global $g_layout;
      
      $g_layout->initGui();
      $g_layout->ui_top($title);
                  
      $form = '<b>'.$title.'</b><br><br>
               <form action="setup.php">'.
               session_form(SESSION_NESTED).
               '<input type="hidden" name="atkaction" value="'.$nextaction.'">'.
               $output;
       
      if ($nextaction!="")
      {                        
        $form.= '<br><br><input type="submit" value="'.($nextaction!=$this->m_action?"Continue":"Retry").'"><br><br>';
      }
               
      $form.= '</form>';
      $g_layout->output($form);
      $g_layout->ui_bottom();
      
    }
    
    /**
     * Check what version of the Achievo database we are using.
     */
    function action_versioncheck()
    {         
      // Check if we have a connection first.
      $res = $this->getDbStatus();
      
      if ($res == DB_SUCCESS)
      {
        global $g_db;
        $tables = $g_db->table_names();
    
        if (count($tables)==0)
        {
          return $this->setupScreen("Database appears to be empty",
                                    "Your database connection is working. However, the specified database '".atkconfig("databasename")."' seems to be empty.
                                     <br>This means I need to install a new database. 
                                     <br><br><i>If you think this is not correct, please verify your database setup, and rerun this script.</i>
                                     <br><br>To continue with the installation of a new Achievo database, please click 'Continue'.",
                                    "installdb");
        }
        else
        {
          return $this->setupScreen("There is a database, go check which version",
                                    "lalala");
        }                
      }
      else
      {
        return $this->handleDbStatus($res);
      }
    }
    
    function action_installdb()
    {
      global $g_modules, $g_layout;
      
      $resultmsg = array("ok" => "Installation completed",
                         "failed" => "Installation failed",
                         "faileddep" => "Not installed (dependencies could not be resolved)",
                         "cyclicdep" => "Not installed (Cyclic dependency detected)",
                         "alreadyinstalled" => "No installation necessary, module was already installed",
                         "notfound" => "Not installed (Module could not be found)");
                         
      
      // We loop through the modules to install them. 
      // The setup module contains essential table needed for the installation process,
      // so we install this first.
      $result = array();
      
      if ($this->installModule("setup", $result))
      {
      
        foreach ($g_modules as $modname=>$moduledir)
        {
          if ($modname!="setup") // we already installed setup when we started.
          {
            if (!$this->installModule($modname, $result, true))
            {
              atkdebug("Installation failed.");
              break;
            }
          }
        }        
      }
      else
      {
        $output = "I was unable to install the system table, so I cannot continue with the installation. Please verify your database setup and try again.";
      }
      
      if (count($result)>0)
      {
        $g_layout->initGui();
        $output = $g_layout->data_top();
        $output.= $g_layout->tr_top();
        $output.= $g_layout->ret_td_datatitle("Module");
        $output.= $g_layout->ret_td_datatitle("Result");
        $output.= $g_layout->tr_bottom();
        foreach ($result as $modname => $result)
        {
          $output.= $g_layout->tr_top();
          $output.= $g_layout->ret_td($modname);          
          $output.= $g_layout->ret_td((!empty($resultmsg[$result])?$resultmsg[$result]:$result));
          $output.= $g_layout->tr_bottom();
        }
        $output.= $g_layout->data_bottom();
      }
      return $this->setupScreen("Installation result", $output);
    }
    
    /**
     * Install a module, and if dependencies on other modules exist, install
     * those modules as well or fail with an error message.
     *
     * @param $modname The name of the module to install.
     * @param $result  This is an array in which the method will store the
     *                 result of the installation, using the modulename as
     *                 array key.
     * @param $resolvedeps If set to false, the installModule function will
     *                     break if dependencies are not met. If set to true
     *                     It will try to install the modules the requested
     *                     module depends on.
     */
    function installModule($modname, &$result, $resolvedeps=false)
    {
      atkdebug("installModule call for $modname");
      // Cycle protection.
      if ($result[$modname]=="ok")
      {
        // Module is already installed. 
        atkdebug("Module ".$modname." already installed in this run.");
        return true;
      }
      else if ($result[$modname]=="faileddep")
      {
        // Module failed dependencies before, and now apparently, during the 
        // resolvement of its dependencies, some other module requested it
        // to be installed again. This will lead to a loop, so we must
        // stop the installation.
        $result[$modname] = "cyclicdep";
        atkdebug("Module ".$modname." has cyclical dependency.");
        return false;
      }
    
      // Retrieve already installed modules. We are currently only interested
      // in the names of the installed modules, not the versions.
      $installed_mods = array_keys($this->getInstalledModules());
      
      // Yet another check. The requested module may already have been
      // installed before. 
      if (in_array($modname, $installed_mods))
      {
        $result[$modname] = "allreadyinstalled";
        atkdebug("Module ".$modname." already installed in previous run.");
        return true;
      }
      
      $module = getModule($modname); 
      if (!is_object($module))
      {
        $result[$modname] = "notfound";
        atkdebug("Module ".$modname." could not be found.");
        return false;
      }
      
      $deps = array();
      
      // if method doesn't exist, assume that there are no dependencies.
      // Note that everything derived from achievoModule should have this 
      // method. Therefor, absence indicates that this module might not be
      // an achievo module.
      if (method_exists($module, "getDependencies"))       
      {
        $deps = $module->getDependencies(DEP_INSTALL);
      }
      else
      {
        atkdebug("Warning: $modname might not be an Achievo module.");
      }
      
      for ($i=0, $_i=count($deps); $i<$_i; $i++)
      {
        if (!in_array($deps[$i], $installed_mods))
        {
          // Dependency not met.
          atkdebug("Resolving dependency ".$deps[$i]." for $modname");
          $result[$modname] = "faileddep";
          if ($resolvedeps)
          {
            if (!$this->installModule($deps[$i], $result, true))
            {
              atkdebug("Stopping install of $modname. Resolving dependencies failed");
              return false; // dependencies coult not be resolved. Give up.
            }
          }
          else
          {
            atkdebug("Stopping install of $modname. Dependencies not met");
            return false;
          }
        }
      }
      
      // if we get here, all dependencies are either resolved, or we have given up already
      // and returned false.      
      
      // if we get here, everything should be installed.
      atkdebug("Installed $modname");
      $result[$modname] = "ok";
      return true;
    }
    
    /**
     * This function retrieves the list of installed modules. 
     *
     * @return An array, where the modulename is the key, and the current
     *         installed version of the module is the value.
     */
    function getInstalledModules()
    {
      // (Ivo) Mental note: I originally had in mind to cache the result of the 
      // query, so subsequent calls would not need to perform a query. 
      // But I didn't implement this, since modules may be installed or
      // deinstalled between 2 calls.
      $result = array();
      $versioninfo = getNode("setup.versioninfo");
      
      if (is_object($versioninfo)) // If this object doesn't exist, we don't have
                                   // a correct installer, so we can only return 
                                   // an empty  result.
      {       
        $rows = $versioninfo->selectDb();
        for ($i=0, $_i=count($rows); $i<$_i; $i++)
        {
          $result[$rows[$i]["module"]] = $rows[$i]["version"];
        }
      }
      return $result;
    }
        
  }
  
?>