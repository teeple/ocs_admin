<?php

function ocs_admin_campaign_export( $form, &$form_state)
{
    $form['desc'] = array(
        '#type' => 'markup',
        '#markup' => '<p>Check the Rule file contents that contains the active campaign rules.</p>',
    );

    $form['ocs'] = array(
        '#type' => 'fieldset',
        '#title' => 'OCS(Online Charging System) Access Info',
        '#description' => t('OCS Access Settings. It can be changed ').
            l( t('here'), '/admin/config/system/ocs_admin'),
        '#collapsible' => TRUE, 
        '#collapsed' => FALSE,
    );

    $ip_address = variable_get('ocs_admin_ip_address', 'Administrative Warning');
    $user_account = variable_get('ocs_admin_user_account', 'Administrative Warning');
    $user_password = variable_get('ocs_admin_user_password', 'Administrative Warning');
    $config_directory = variable_get('ocs_admin_config_directory', 'Administrative Warning');
    $action_file_name = variable_get('ocs_admin_ocs_event_action_rule_file', 'Administrative Warning');
    $bonus_file_name = variable_get('ocs_admin_ocs_activation_bonus_file', 'Administrative Warning');

    $form['ocs']['ip_address_markup'] = array(
        '#type' => 'item',
        '#title' => 'IP Address',
        '#markup' => $ip_address,
    );

    $form['ocs']['user_account'] = array(
        '#type' => 'item',
        '#title' => 'Account Name',
        '#markup' => $user_account,
    );

    $form['ocs']['config_directory'] = array(
        '#type' => 'item',
        '#title' => 'Configuration Directory',
        '#markup' => $config_directory,
    );

    $form['ocs']['event_action_file'] = array(
        '#type' => 'item',
        '#title' => 'Event Action Rule File Name',
        '#markup' => $action_file_name,
    );

    $form['ocs']['bonus_rule_file'] = array(
        '#type' => 'item',
        '#title' => 'Activation Bonus Rule File Name',
        '#markup' => $bonus_file_name,
    );

    list( $action, $bonus) = _campaign_export();

    $form['action_rule'] = array(
        '#type' => 'textarea',
        '#title' => 'event_actions.rule',
        '#value' => $action,
        '#rows' => 10,
    );

    $form['bonus_rule'] = array(
        '#type' => 'textarea',
        '#title' => 'activation_bonus.rule',
        '#value' => $bonus,
        '#rows' => 10,
    );

    $form['actions'] = array('#type' => 'actions');

    $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Upload Rule Files'),
    );

    $form['actions']['cancel'] = array(
        '#type' => 'submit',
        '#value' => t('Cancel'),
    );

    return $form;
}

function ocs_admin_campaign_export_submit( $form, &$form_state)
{
    $values = $form_state['values'];

    if ( $values['op'] != 'Cancel') {

        // write to file
        try{
            $ip_address = variable_get('ocs_admin_ip_address', 'Administrative Warning');
            $user = variable_get('ocs_admin_user_account', 'Administrative Warning');
            $passwd = variable_get('ocs_admin_user_password', 'Administrative Warning');
            $dir = variable_get('ocs_admin_config_directory', 'Administrative Warning');

            $files = array(
                array(
                    'fname' => variable_get('ocs_admin_ocs_event_action_rule_file', 'Administrative Warning'),
                    'value' => $values['action_rule'],
                    ),
                array(
                    'fname' => variable_get('ocs_admin_ocs_activation_bonus_file', 'Administrative Warning'),
                    'value' => $values['bonus_rule'],
                    ));

            foreach( $files as $file) {
                $fd = fopen( '/tmp/'. $file['fname'], 'w');
                fwrite( $fd, $file['value']);
                fclose( $fd);

                // ftp to host
                if ( ocs_admin_ftp_put( $ip_address, $user, $passwd, $dir, $file['fname'])) {
                    drupal_set_message( t('Rule file "@name" is updated in the OCS', array( '@name' => $file['fname'])));
                }
            }
        }
        catch( Exception $e) {
            drupal_set_message( $e->getMessage(), 'warning');       
        }
    }

    $form_state['redirect'] = 'ocs_admin/campaign/';
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
    //dpm( $campaigns, 'campaigns');
    
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
!rules

[ACTIVATION_BONUS_CONDITION]
';

    $bonus_amount = '
[RULESET_ACTIVATION_BONUS_AMOUNT]
targetRule , string(32)
db_action , string(128)
!rules
 
[ACTIVATION_BONUS_AMOUNT]
';

    $bonus_rules = array();
    $amount_rules = array();

    $newline = '
';

    $ocs_action = '';

    foreach( $campaigns as $campaign) {
        $campaign_id = $campaign['campaign_id'];
        $events = ocs_admin_campaign_events_load_all( $campaign_id, 'event_id');
        $conditions = ocs_admin_campaign_conditions_load_all( $campaign_id);
        $actions = ocs_admin_campaign_actions_load_all( $campaign_id);

        //dpm( $events, 'events');
        //dpm( $conditions, 'conditions');
        //dpm( $actions, 'actions');

        // build event_actions.rule file
        foreach( array_keys($events) as $event_id) {
            $event_node = node_load( $event_id);

            $property = json_decode($event_node->field_property['und'][0]['value'], TRUE);
            $property_str = '';
            foreach( $property as $key => $value) {
                $property_str .= $key . '=' . $value . ' ';
            }

            $event_action .= t("
@id: @property event_name=@event, promotion_service_id = giving_bonus , priority = 10 , promotion_parameter = 't_bonus_condition=activation_bonus_condition&t_bonus_amount=activation_bonus_amount'", 
                array( '@id' => $campaign_id, '@event' => $event_node->title,
                    '@property' => isset( $property) ? $property_str.',' : ''));

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

                        list( $factor, $op, $value) = explode( ' ', $cond['cond_rule']);

                        if ( ! isset( $bonus_rules[$factor])) {
                            $bonus_rules[ $factor] = _get_rule_property( $cond['param_id'], $factor);
                        }
                    }
                }

                if ( isset( $actions[ $rule_id])) {

                    $i =0;
                    foreach( $actions[ $rule_id] as $action) {
                        $param = ocs_admin_campaign_action_param_load( $action['action_id']);

                        if ( $action['ocs_action_id'] == $action_give_bonus) {
                            // build parameter string : bonus_amount=123,counter_id=0,
                            $param_str = '';
                            foreach( $param as $para) {
                                $factor = $para['param_name'];
                                $param_str .= t('@n=@v,',
                                    array( '@n' => $factor, '@v' => _get_action_param_value( $para)));

                                if ( ! isset( $amount_rules[$factor])) {
                                    $amount_rules[$factor] = _get_rule_property( $cond['param_id'], $factor);
                                }
                            }

                            $bonus_amount .= $newline . t('@name : targetRule = @name, ',
                                array( '@name' => $rule_name)) . $param_str;
                        }
                        else {
                            // build parameter string in this format : send_noti&msgid=123&counter_id=0,
                            $param_str = '';
                            foreach( $param as $para) {
                                $param_str .= t('&@n=@v',
                                    array( '@n' => $factor, 
                                        '@v' => _get_action_param_value( $para)));
                            }

                            $node = node_load( $action['ocs_action_id']);
                            $bonus_amount .= $newline . t("@name : targetRule = @name, db_action='@action!param'",
                                array( '@name' => $rule_name, '@action' => $node->title,
                                    '!param' => $param_str));
                        }

                    }
                }
            }
        }


    }

//    dpm( $bonus_rules, 'rules');
    $rule_str = '';
    foreach( $bonus_rules as $key => $property) {
        $rule_str .= $key . ', ' . $property . $newline;
    }
    $bonus_cond = t( $bonus_cond, array('!rules' => $rule_str));

    $rule_str = '';
    foreach( $amount_rules as $key => $property) {
        $rule_str .= $key . ', ' . $property . $newline;
    }
    $bonus_amount = t( $bonus_amount, array('!rules' => $rule_str));
    
    return array( $event_action, $bonus_cond. $bonus_amount);
}

function _get_rule_property( $param_id, $factor)
{
    // return 'int' for account counters
    if ( substr( $param_id, -7) == 'account') {
        return 'int ';
    }

    $items = entity_load( 'node', FALSE, array( 'type' => 'ocs_param', 'title' => $factor));
    $item = array_shift( $items);
    if ( ! empty($item)) {
        return $item->field_rule_property['und'][0]['value'];
    }
    return '';
}

function _get_action_param_value( $param)
{
    $counter_id = ocs_admin_get_counter_id_list();

    if ( $param['param_name'] == 'counter') {
        // return counter id
        if ( ! isset( $counter_id[ $param['value']])) {
            drupal_set_message( t('Unable to find counter "@name" in the counter name table.', array( '@name' => $param['value'])), 'warning');
        }
        else {
            return $counter_id[ $param['value']];
        }
    }
    return $param['value'];
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
