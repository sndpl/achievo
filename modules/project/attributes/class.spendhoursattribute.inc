<?php
/**
 * This file is part of the Achievo distribution.
 * Detailed copyright and licensing information can be found
 * in the doc/COPYRIGHT and doc/LICENSE files which should be
 * included in the distribution.
 *
 * @package achievo
 * @subpackage project
 *
 * @copyright (c)2008 Ibuildings B.V.
 * @license http://www.achievo.org/licensing Achievo Open Source License
 *
 * @version $Revision$
 * $Id$
 */
  include_once("achievotools.inc"); // for time_format

 /**
  * Small attribute to calculate the amount of spend hours
  * on a phase
  */
/**
 * Phase dependency attribute
 *
 * @package achievo
 * @subpackage project
 */
  class spendHoursAttribute extends atkAttribute
  {
    /**
     * The field that contains the phaseid
     * @var int
     */
    var $m_phaseid_field = "";

    /**
     * Constructor
     *
     * @param string $name Name of the attribute
     * @param int $flags Flags of the attribute
     */
    function spendHoursAttribute($name,$phaseid_field,$flags=0)
    {
      $this->m_phaseid_field = $phaseid_field;
      $this->atkAttribute($name, $flags|AF_HIDE_SEARCH|AF_READONLY|AF_HIDE_ADD);
    }

    /**
     * Load values
     *
     * @param atkDb $db pointer to the database
     * @param array $record Record
     * @param string $mode Mode for loading (admin, select, copy, etc)
     * @return int
     */
    function load(&$db, $record, $mode="")
    {
      $phaseattrib = $this->m_ownerInstance->m_attribList[$this->m_phaseid_field];

      $sql = "SELECT sum(time) as spend_minutes FROM hoursbase WHERE phaseid=".$phaseattrib->value2db($record);
      if(isset($record["personid"]["id"])) $sql.=" AND userid='".$record["personid"]["id"]."'";
      $nrow = $db->getrows($sql);
      return ($nrow[0]["spend_minutes"]/60);
    }

    /**
     * Display override. If possible, displays a link to the hoursurvey to view details.
     * @param array $record Record
     * @param string $mode mode
     * @return string
     */
    function display($record, $mode)
    {
      $value = $record[$this->fieldname()];
      $org = time_format(($value*60), true);
      if(moduleExists('reports'))
      {
        if ($value>0 && atkArrayNvl($record, $this->m_phaseid_field, "")!="")
        {
          /* @var $phaseattrib atkAttribute */
          $phaseattrib = $this->m_ownerInstance->m_attribList[$this->m_phaseid_field];
          $params["phaseid"] = "phase.id='".$phaseattrib->value2db($record)."'";
          $params["startdate"] = "2000-01-01";
          $params["enddate"] = date("Y-m-d");
          if(isset($record["personid"]["id"]))
          {
            $params["userid"] = $record["personid"]["id"];
          }
          else
          {
            $params["userid"] = "all";
          }

          $org = href(dispatch_url("reports.hoursurvey", "report", $params), $org, SESSION_NESTED, ($mode!="list"));
        }
      }
      return $org;
    }

    /**
     * No function, but is neccesary
     *
     * @param atkDb $db atkDb
     * @param array $record Record
     * @param string $type Type
     * @return boolean
     */
    function store($db, $record, $type)
    {
      return true;
    }

    /**
     * No function, but is neccesary
     *
     * @param atkQuery $query ATK Query object
     * @param string $table Tablename
     * @param string $aliasprefix Alias prefix
     * @param array $rec Record
     * @param int $level Level
     */
    function addToQuery(&$query, $tablename="", $fieldaliasprefix="",$level)
    {
    }
  }

?>