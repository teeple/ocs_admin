<?php

class OcsCampaignRules
{
    private $rules;

    function OcsCampaign() {
    }

    function load( $campaign_id) 
    {
        db_set_active('ocsdb');

        // load all rules data
        $query = db_select( 'ocs_rules', 'c');
        $result = $query->condition( 'c.rule_key', t('campaign:!id%', array( '!id' => $campaign_id)), 'like')
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
