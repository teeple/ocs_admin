<?php

class OcsRule {
    private $rule;
    private $conditions;
    private $actions;

    private function load_conditions()
    {
        if ( ! empty($this->rule)) {
            // load all conditions
            $query = db_select( 'ocs_campaign_conditions', 'c');
            $result = $query->condition( 'c.rule_id', $this->rule->rule_id, '=')
                ->fields( 'c')
                ->execute();

            $this->conditions = array();
            while( $record = $result->fetch()) 
            {
                $this->conditions[] = $record;
            }
        }
    }

    private function load_actions()
    {
        if ( !empty( $this->rule)) {
            // load action list
            $query = db_select( 'ocs_campaign_actions', 'c');
            $result = $query->condition( 'c.rule_id', $this->rule->rule_id, '=')
                ->fields( 'c')
                ->execute();

            $this->actions = array();
            while( $record = $result->fetch())
            {
                $this->actions[] = $record;
            }
        }
    }


    // constructor
    function OcsRule( $instance = NULL) {
        $this->rule = $instance;

        if ( $instance) {
            $this->load_conditions();
            $this->load_actions();
        }
    }

    function load( $rule_id) {
        db_set_active( 'ocsdb');

        $query = db_select( 'ocs_campaign_rules', 'c');
        $result = $query->condition( 'c.rule_id', $rule_id, '=')
            ->fields( 'c')
            ->execute()
            ->fetch();

        $this->rule = $result;
        $this->load_conditions();
        $this->load_actions();
     
        db_set_active( 'default');
    }

    function event_id() 
    {
        return isset( $this->rule) ? $this->rule->event_id : NULL;
    }

    function campaign_id()
    {
        return isset( $this->rule) ? $this->rule->campaign_id : NULL;
    }

    function rule_name()
    {
        return isset( $this->rule) ? $this->rule->rule_name : NULL;
    }

    function conditions()
    {
        return $this->conditions;
    }

    function actions()
    {
        return $this->actions;
    }

    function isNull() {
        return $this->rule == NULL;
    }

}

class OcsCampaign
{
    private $rules;

    function OcsCampaign() {
    }

    function load( $campaign_id) 
    {
        db_set_active('ocsdb');

        // load all rules data
        $query = db_select( 'ocs_campaign_rules', 'c');
        $result = $query->condition( 'c.campaign_id', $campaign_id, '=')
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
}
