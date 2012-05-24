<?php

class OcsRule {
    var $rule;
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
                $this->conditions[$record->cond_id][] = $record;
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

        $query = db_select( 'ocs_rules', 'c');
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
        // key format :    campaign:<id>:event:<id>
        $list = explode( ':', $this->rule->rule_key);
        if ( count( $list) == 4 && $list[2] == 'event') return $list[3];
        return NULL;
    }

    function campaign_id()
    {
        $list = explode( ':', $this->rule->rule_key);
        if ( count( $list) == 4 && $list[0] == 'campaign') return $list[1];
        return NULL;
    }

    function group_id()
    {
        $list = explode( ':', $this->rule->rule_key);
        if ( $list[0] == 'group') return $list[1];
        return NULL;
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


    private function get_operations( $q_arg = NULL) 
    {
        $form = array();

		$operations = array();
		$t_arg = array( '@rule' => $this->rule->rule_id);
        $q_arg['query']['rule_name'] = $this->rule->rule_name;
        $q_arg['query']['redirect'] = current_path();
        $q_arg['query']['rule_key'] = $this->rule->rule_key;

        $operations['add'] = array( 
            '#type' => 'link',
            '#title' => t('Add'),
            '#href' => t( 'ocs_admin/rule/add'),
            '#options' => $q_arg,
        );
        
		$operations['delete'] = array( 
		    '#type' => 'link',
		    '#title' => t('Delete'),
		    '#href' => t( 'ocs_admin/rule/@rule/delete', $t_arg),
            '#options' => $q_arg,
		  );

		$operations['edit'] = array( 
		    '#type' => 'link',
		    '#title' => t('Edit'),
		    '#href' => t( 'ocs_admin/rule/@rule/edit', $t_arg),
            '#options' => $q_arg,
		  );

        return $operations;
    }

    private function get_cond_operations( $cond, $t_arg, $q_arg)
    {
        $t_arg['@cond'] = $cond->cond_id;
        $q_arg['query']['name'] = $cond->cond_name;

        $operations = array();

        // 'cond_idx' value must not be passed to 'Add AND' link
        $operations['add'] = array( 
            '#type' => 'link',
            '#title' => t('Add AND'),
            '#href' => t( 'ocs_admin/rule/@rule/cond/@cond/add', $t_arg),
            '#options' => $q_arg,
        );

        // 'cond_idx' value must not be passed to 'Add AND' link
        $q_arg['query']['idx'] = $cond->cond_idx;
        $operations['delete'] = array( 
            '#type' => 'link',
            '#title' => t('Delete'),
            '#href' => t( 'ocs_admin/rule/@rule/cond/@cond/delete', $t_arg),
            '#options' => $q_arg,
        );

        $operations['edit'] = array( 
            '#type' => 'link',
            '#title' => t('Edit'),
            '#href' => t( 'ocs_admin/rule/@rule/cond/@cond/edit', $t_arg),
            '#options' => $q_arg,
        );

        return $operations;
    }

    private function render_conditions( $q_arg = NULL)
    {
        $rows = array();
        // get conditions
        $idx = 0;
        $white_cell = array( 'data' => '', 'class' => array( 'white-cell'));
        $no_cond = count($this->conditions);
        $t_arg = array( '@campaign' => $this->campaign_id(), '@rule' => $this->rule->rule_id);

//        dpm( $this->conditions, 'conds');
        foreach( $this->conditions as $conds) {
            // add 'condition OR'
            if ( $idx ++ == 0) {
                $operations = array();
                $operations['add'] = array( 
                    '#type' => 'link',
                    '#title' => t('Add OR'),
                    '#href' => t( 'ocs_admin/rule/@rule/cond/0/add', $t_arg),
                    '#options' => $q_arg,
                );

                $operations['delete'] = array( 
                    '#type' => 'link',
                    '#title' => t('Delete OR'),
                    '#href' => t( 'ocs_admin/rule/@rule/cond/0/delete', $t_arg),
                    '#options' => $q_arg,
                );

                $operations['edit'] = array( 
                    '#type' => 'item',
                );

                $row = array(
                      $white_cell,
                      array( 'data' => 'Conditions (OR)', 'colspan' => '2', ));
                $row = array_merge( $row, $this->render_operations( $operations));
                $rows[] = $row;
            }

            $and_idx =0;
            foreach( $conds as $cond) {

                $row = array();
                $row[] = $white_cell;
                if ( $and_idx ++ == 0) {
                    $row[] = array( 'data' => $this->indentation(1) . 'AND', 'rowspan' => count( $conds));
                }

                $info = array(
                    '#type' => 'item',
                    '#title' => $cond->cond_name,
                );

                $row[] = array( 'data' => $info, 
                  'class' => ($and_idx == count($conds)) ? array( 'bd-btm', 'dotted') : array());

                $operations = $this->get_cond_operations( $cond, $t_arg, $q_arg);
                $row = array_merge( $row, $this->render_operations( $operations));
                $rows[] = $row;
            }
        }

        if ( $no_cond == 0) {
            $t_arg = array( '@campaign' => $this->campaign_id(), '@rule' => $this->rule->rule_id);
            $rows[] = array( 
                $white_cell,
                array('data' => 'Conditions'),
                t('There are no conditions yet. <a href="@url">Add Conditions</a>.',
                    array( '@url' => url( t( 'ocs_admin/rule/@rule/cond/0/add', $t_arg), 
                        $q_arg))),
            );
        }

        return $rows;
    }

    private function get_action_operations( $action, $t_arg, $q_arg)
    {
        $t_arg['@action'] = $action->action_id;
        $q_arg['query']['name'] = $action->action_name;

        $operations = array();

        $operations['add'] = array( 
            '#type' => 'item',
            '#value' => '',
        );

        $operations['delete'] = array( 
            '#type' => 'link',
            '#title' => t('Delete'),
            '#href' => t( 'ocs_admin/rule/@rule/action/@action/delete', $t_arg),
            '#options' => $q_arg,
          );

        $operations['edit'] = array( 
            '#type' => 'link',
            '#title' => t('Edit'),
            '#href' => t( 'ocs_admin/rule/@rule/action/@action/edit', $t_arg),
            '#options' => $q_arg,
          );

        return $operations;
    }

    private function render_actions( $q_arg = NULL)
    {
        $rows = array();
        $white_cell = array( 'data' => '', 'class' => array( 'white-cell'));
        $no_action = count( $this->actions);
        $t_arg = array( '@campaign' => $this->campaign_id(), '@rule' => $this->rule->rule_id);

//        dpm( $this->actions, 'actions');

        // add actions
        $idx = 0;
        foreach( $this->actions as $action_id => $action) {
            if ( $idx ++ == 0) {
                // add operations for 'Action'
                $operations = array();
                $operations['add'] = array( 
                    '#type' => 'link',
                    '#title' => t('Add'),
                    '#href' => t( 'ocs_admin/rule/@rule/action/add', $t_arg),
                    '#options' => $q_arg,
                );

                $operations['delete'] = array( 
                    '#type' => 'link',
                    '#title' => t('Delete All'),
                    '#href' => t( 'ocs_admin/rule/@rule/action/0/delete', $t_arg),
                    '#options' => $q_arg,
                );
                $row = array(
                    $white_cell,
                    array( 'data' => 'Actions', 'colspan' => '2', 'class' => array('bd-top')));
                $row = array_merge( $row, $this->render_operations( $operations));
                $rows[] = $row;
            }

            $row = array();
            $row[] = $white_cell;
            $row[] = $white_cell;

            $title = substr( $action->action_name, 0, 70);
            if ( strlen( $action->action_name) > 70) $title .= ' ...';
            $row[] = array( 'data' => array(
                    '#type' => 'item',
                    '#title' => $title,
                ));

            $operations = $this->get_action_operations( $action, $t_arg, $q_arg);
            $row = array_merge( $row, $this->render_operations( $operations));
            $rows[] = $row;
        }

        if ( $no_action == 0) {
    //        dpm( $form['#rule'][$rule_id], 'rule');

            $row = array();
            $row[] = $white_cell;
            $row[] = array('data' => 'Actions', 'class' => array('bd-top', 'dotted'));
            $row[] = array('data' => 
                t('There are no actions yet. <a href="@url">Add Actions</a>.',
                    array( '@url' => url( t( 'ocs_admin/rule/@rule/action/add', $t_arg), $q_arg))),

                    'class' => array( 'bd-top', 'dotted'));
            $row[] = array( 'data' => '', 'colspan' => '3');

            $rows[] = $row;
        }

        return $rows;
    }

    private function indentation( $tabs) 
    {
        $output = '';
        for( $i =0; $i < $tabs; $i++) {
            $output .= '<div class="indentation"></div>';
        }
        return $output;
    }

    private function render_operations( $list, $class = array())
    {
        $operations = array();
        foreach( $list as $op) {
          $operations[] = array( 'data' => drupal_render($op),
            'class' => array_merge( array('menu-operations'), $class));
        }

        return $operations;
    }


    function table_rows( $arg = array())
    {
        $rows = array();

        if ( empty( $this->rule)) return $rows;

        // first, render rule & operations
        $row = array();
        $row[] = array( 'data' => t('Rule @id : @name', 
            array( '@id' => $this->rule->rule_id, '@name' => $this->rule->rule_name)),
            'colspan' => '3', 'class' => array( 'bd-top'));

        $operations = $this->get_operations( $arg);
        $rows[] = array_merge( $row, $this->render_operations( $operations, array( 'bd-top')));

        $arg['query']['rule_name'] = $this->rule->rule_name;
        $arg['query']['redirect'] = current_path();

        // add conditions
        $rows = array_merge( $rows, $this->render_conditions( $arg));

        // add actions
        $rows = array_merge( $rows, $this->render_actions( $arg));

        return $rows;
    }


    function get_rule_script( $target_action, $optional_rule = NULL)
    {
        $newline = '
';
        $cond_rules = '';
        $cond_factors = array();
        $action_rules = '';
        $action_factors = array();


        $rule_name = str_replace( ' ', '_', $this->rule->rule_name);
        if ( ! empty( $this->conditions)) {
            foreach( $this->conditions as $conds) {
                // build condition script
                $cond_rules .= $newline . $rule_name . ' : ';

                $i =0;
                foreach( $conds as $cond) {
                    if ( $i ++ > 0) $cond_rules .= ',';
                    $cond_rules .= $cond->cond_rule;

                    list( $factor, $op, $value) = explode( ' ', $cond->cond_rule);

                    if ( ! isset( $cond_factors[$factor])) {
                        $cond_factors[ $factor] = _get_rule_property( $cond->param_id, $factor);
                    }
                }

                // add optional rules such as 'vpn_group_id=10'
                if ( isset( $optional_rule)) $cond_rules .= ', ' . $optional_rule;
            }
        }

        if ( ! empty( $this->actions)) {

            $i =0;
            foreach( $this->actions as $action) {
                $param = ocs_admin_campaign_action_param_load( $action->action_id);

                if ( $action->ocs_action_id == $target_action) {
                    // build parameter string : bonus_amount=123,counter_id=0,
                    $param_str = '';
                    foreach( $param as $para) {
                        $factor = $para['param_name'];
                        $param_str .= t('@n=@v,',
                            array( '@n' => $factor, '@v' => _get_action_param_value( $para)));

                        if ( ! isset( $action_factors[$factor])) {
                            $action_factors[$factor] = _get_rule_property( $cond->param_id, $factor);
                        }
                    }

                    $action_rules .= $newline . t('@name : targetRule = @name, ',
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

                    $node = node_load( $action->ocs_action_id);
                    $action_rules .= $newline . t("@name : targetRule = @name, db_action='@action!param'",
                        array( '@name' => $rule_name, '@action' => $node->title,
                            '!param' => $param_str));
                }

            }
        }

        return array(
            'conditions' => array( 'factors' => $cond_factors, 'rules' => $cond_rules),
            'actions' => array( 'factors' => $action_factors, 'rules' => $action_rules),
        );
     }

     static function get_action( $title) 
     {
        $items = entity_load( 'node', FALSE, array( 'type' => 'ocs_actions', 'title' => $title));
        return array_shift( $items);
     }
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


