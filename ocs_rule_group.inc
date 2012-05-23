<?php

require_once( 'ocs_rule.inc');

class OcsGroupRules
{
    private $rules;

    function load( $group_id) 
    {
        db_set_active('ocsdb');

        // load all rules data
        $query = db_select( 'ocs_rules', 'c');
        $result = $query->condition( 'c.rule_key', t('group:!id:%', array( '!id' => $group_id)), 'like')
            ->fields( 'c')
            ->execute();

        $this->rules = array();
        while( $record = $result->fetch())
        {
            $this->rules[ $record->rule_id] = new OcsRule( $record);
        }

        db_set_active( 'default');
    }

    function rules()
    {
        return $this->rules;
    }

    function render()
    {
        $rows = array();

        foreach( $this->rules as $rule) {
            $rows = array_merge( $rows, $rule->table_rows());
        }

        $header = array(
            t('Rules'),
            array( 'data' => t('Items'), ),
            array( 'data' => t('Description')),
            array( 'data' => t('Operations'), 'colspan' => '3'),
        );

        return theme( 'table', array( 'header' => $header, 'rows' => $rows, 'attributes' => array( 'id' => 'menu-overview')));
    }
}
