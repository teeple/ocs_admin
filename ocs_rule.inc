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


    private function get_operations( $q_arg = NULL) 
    {
        $form = array();

		$operations = array();
		$t_arg = array( '@campaign' => $this->rule->campaign_id, '@rule' => $this->rule->rule_id, 
            '@event' => $this->rule->event_id);
        $q_arg['query']['rule_name'] = $this->rule->rule_name;

        $operations['add'] = array( 
            '#type' => 'link',
            '#title' => t('Add'),
            '#href' => t( 'ocs_admin/campaign/@campaign/event/@event/rule/add', $t_arg),
            '#options' => $q_arg,
        );
        
		$operations['delete'] = array( 
		    '#type' => 'link',
		    '#title' => t('Delete'),
		    '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/delete', $t_arg),
            '#options' => $q_arg,
		  );

		$operations['edit'] = array( 
		    '#type' => 'link',
		    '#title' => t('Edit'),
		    '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/edit', $t_arg),
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
            '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/cond/@cond/add', $t_arg),
            '#options' => $q_arg,
        );

        // 'cond_idx' value must not be passed to 'Add AND' link
        $q_arg['query']['idx'] = $cond->cond_idx;
        $operations['delete'] = array( 
            '#type' => 'link',
            '#title' => t('Delete'),
            '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/cond/@cond/delete', $t_arg),
            '#options' => $q_arg,
        );

        $operations['edit'] = array( 
            '#type' => 'link',
            '#title' => t('Edit'),
            '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/cond/@cond/edit', $t_arg),
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
        $t_arg = array( '@campaign' => $this->rule->campaign_id, '@rule' => $this->rule->rule_id);

//        dpm( $this->conditions, 'conds');
        foreach( $this->conditions as $conds) {
            // add 'condition OR'
            if ( $idx ++ == 0) {
                $operations = array();
                $operations['add'] = array( 
                    '#type' => 'link',
                    '#title' => t('Add OR'),
                    '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/cond/0/add', $t_arg),
                    '#options' => $q_arg,
                );

                $operations['delete'] = array( 
                    '#type' => 'link',
                    '#title' => t('Delete OR'),
                    '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/cond/0/delete', $t_arg),
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
            $t_arg = array( '@campaign' => $this->rule->campaign_id, '@rule' => $this->rule->rule_id);
            $rows[] = array( 
                $white_cell,
                array('data' => 'Conditions'),
                t('There are no conditions yet. <a href="@url">Add Conditions</a>.',
                    array( '@url' => url( t( 'ocs_admin/campaign/@campaign/rule/@rule/cond/0/add', $t_arg), 
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
            '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/action/@action/delete', $t_arg),
            '#options' => $q_arg,
          );

        $operations['edit'] = array( 
            '#type' => 'link',
            '#title' => t('Edit'),
            '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/action/@action/edit', $t_arg),
            '#options' => $q_arg,
          );

        return $operations;
    }

    private function render_actions( $q_arg = NULL)
    {
        $rows = array();
        $white_cell = array( 'data' => '', 'class' => array( 'white-cell'));
        $no_action = count( $this->actions);
        $t_arg = array( '@campaign' => $this->rule->campaign_id, '@rule' => $this->rule->rule_id);

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
                    '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/action/add', $t_arg),
                    '#options' => $q_arg,
                );

                $operations['delete'] = array( 
                    '#type' => 'link',
                    '#title' => t('Delete All'),
                    '#href' => t( 'ocs_admin/campaign/@campaign/rule/@rule/action/0/delete', $t_arg),
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
            $row[] = array( 'data' => array(
                    '#type' => 'item',
                    '#title' => $action->action_name,
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
                    array( '@url' => url( t( 'ocs_admin/campaign/@campaign/rule/@rule/action/add', $t_arg), $q_arg))),

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


    function render( $arg = NULL)
    {
        $rows = array();

        // first, render rule & operations
        $row = array();
        $row[] = array( 'data' => t('Rule @id : @name', 
            array( '@id' => $this->rule->rule_id, '@name' => $this->rule->rule_name)),
            'colspan' => '3', 'class' => array( 'bd-top'));

        $operations = $this->get_operations( $arg);
        $rows[] = array_merge( $row, $this->render_operations( $operations, array( 'bd-top')));

        if ( isset( $arg)) $arg['query']['rule_name'] = $this->rule->rule_name;

        // add conditions
        $rows = array_merge( $rows, $this->render_conditions( $arg));

        // add actions
        $rows = array_merge( $rows, $this->render_actions( $arg));

        return $rows;
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
