<?php

function ocs_admin_campaign_export( $form, &$form_state)
{
    $form['file'] = array(
        '#type' => 'textarea',
        '#title' => 'event_actions.rule',
        '#value' => _campaign_export(),
        '#rows' => 20,
    );

    return $form;
}


function _campaign_export()
{
    $output = '';

    $items = entity_load( 'node', FALSE, array( 'type' => 'ocs_actions', 'title' => 'give_bonus'));
    if ( empty( $items)) {
        drupal_set_message( 'unable to find action: give_bonus', 'error');
        return $output;
    }

    $action = array_shift( $items);
    $action_give_bonus = $action->nid;

    $campaigns = ocs_admin_campaigns_load_all();
    dpm( $campaigns, 'campaigns');
    
    $event_action = '[RULESET_EVENT_ACTIONS]
service_id , string(30)
event_name , string(32)
promotion_service_id , string(30) , result, mandatory
priority , int , result, mandatory
promotion_category , string(30), result
promotion_parameter, string(256) , result, mandatory';

    $event_action .= '
[EVENT_ACTIONS]';

    $bonus_cond = '

[RULESET_ACTIVATION_BONUS_CONDITION]
fullday, string(10)
product, string(4)
event_name, string(32)

[ACTIVATION_BONUS_CONDITION]
';

    $bonus_amount = '
[RULESET_ACTIVATION_BONUS_AMOUNT]
targetRule , string(32)
counter_id, string(32), result
bonus_amount , int , result
 
[ACTIVATION_BONUS_AMOUNT]
';

    $newline = '
';

    foreach( $campaigns as $campaign) {
        $campaign_id = $campaign['campaign_id'];
        $events = ocs_admin_campaign_events_load_all( $campaign_id, 'event_id');
        $conditions = ocs_admin_campaign_conditions_load_all( $campaign_id);
        $actions = ocs_admin_campaign_actions_load_all( $campaign_id);

        dpm( $events, 'events');
        dpm( $conditions, 'conditions');
        dpm( $actions, 'actions');

        // build event_actions.rule file
        foreach( array_keys($events) as $event_id) {
            $event_node = node_load( $event_id);
            $event_action .= t("
@id: service_id = PC_EVENT, event_name=@event, promotion_service_id = giving_bonus , priority = 10 , promotion_parameter = 't_bonus_condition=activation_bonus_condition&t_bonus_amount=activation_bonus_amount'", 
                array( '@id' => $campaign_id, '@event' => $event_node->title));

            // build activation_bonus.rule file
            foreach( $events[$event_id] as $event) {
                $rule_id = $event['rule_id'];
                $rule_name = str_replace( ' ', '_', $event['rule_name']);
                if ( isset( $conditions[ $rule_id])) {
                    $bonus_cond .= $newline . $rule_name . ' : ';

                    $i =0;
                    foreach( $conditions[ $rule_id] as $cond) {
                        if ( $i ++ > 0) $bonus_cond .= ',';
                        $bonus_cond .= $cond['cond_rule'];
                    }
                }

                if ( isset( $actions[ $rule_id])) {
                    $bonus_amount .= $newline . t('@name : targetRule = @name,',
                        array( '@name' => $rule_name));

                    $i =0;
                    foreach( $actions[ $rule_id] as $action) {
                        if ( $action['ocs_action_id'] == $action_give_bonus) {
                            if ( $i ++ > 0) $bonus_amount .= ',';
                            $bonus_amount .= $action['action_name'];
                        }
                    }
                }
            }
        }


    }

    return $event_action . $bonus_cond . $bonus_amount;
}

// get all active campaigns
function ocs_admin_campaigns_load_all()
{
    db_set_active('ocsdb');

    // get group information
    $query = db_select( 'ocs_campaign', 'c');
    $result = $query->condition( 'c.status', '1', '=')
        ->fields('c')
        ->execute()
        ->fetchAllAssoc( 'campaign_id', PDO::FETCH_ASSOC);

    db_set_active('default');
    return $result;

}
