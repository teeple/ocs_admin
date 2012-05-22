<?php

module_load_include( 'inc', 'ocs_admin', 'ocs_common');

// manages Parameters that belongs to each Event
// 
class OcsEventParam 
{ 
    var $event_id;
    var $params;
    var $options;
    var $selected;
    var $oplist = array(
        OCS_PARAM_TYPE_STRING => array(
            '=', 'starts', 
            ),
        OCS_PARAM_TYPE_INTEGER => array(
            '=', '>', '>=', '<', '<=', 'between',
            ),
        OCS_PARAM_TYPE_ENUM => array(
            '=', 'in list',
            ),
        OCS_PARAM_TYPE_ACCOUNT_KEY => array(
            '=', '>', '>=', '<', '<=', 'between',
            ),
        OCS_PARAM_TYPE_DATE => array(
            '=', '>', '>=', '<', '<=', 'between',
            ),
        OCS_PARAM_TYPE_TIME => array(
            '=', '>', '>=', '<', '<=', 'between',
            ),
        OCS_PARAM_TYPE_PRODUCT_KEY => array(
            '=', 
            ),
    );

    function OcsEventParam()
    {
    }

    function load( $event_id, $rule_id)
    {
        $this->event_id = $event_id;

        $paramlist = array();
        $options = array();

        $node = node_load( $event_id);

        foreach( $node->field_ocs_param_ref['und'] as $param) {
            $para = node_load( $param['target_id']);
    //				dpm( $para, 'parameter');

            // store select options
            $key = $rule_id . ':' . $param['target_id'];
            $value = $para->title;
            $options[ $node->title][ $key] = $value;
            // store parameter info
            $paramlist[$para->nid] = $para->field_ocs_param_type['und'][0]['value'];
            $paramlist[$key] = $value;
            $paramlist[$key.':title'] = $para->title;
            if ( ! isset( $this->selected)) {
                $this->selected = $rule_id . ':' . $param['target_id'];
            }

            // add additional condition if the parameter is 'account key' type
            if ( $para->field_ocs_param_type['und'][0]['value'] == OCS_PARAM_TYPE_ACCOUNT_KEY) {
                $paramlist[$key.':account'] = $options[ $node->title][ $key.':account'] = t( 'counter of the @val party', array( '@val' => $value));
                $paramlist[$para->nid.':account'] = OCS_PARAM_TYPE_ACCOUNT_KEY;
                // reset the type of original parameter as string
                $paramlist[$para->nid] = OCS_PARAM_TYPE_STRING;		
            }
            else if ( $para->field_ocs_param_type['und'][0]['value'] == OCS_PARAM_TYPE_ENUM) {
                // add enum value list
                $paramlist[$key.':enum'] = json_decode( $para->field_ocs_param_value['und'][0]['value'], TRUE);
            }
        }

        $this->params = $paramlist;
        $this->options = $options;
    }

    function operators( $selected)
    {
    //	dpm( $paramlist, 'param');
    //	dpm( $selected, 'selected');

        $type = $this->type( $selected);

        return $this->oplist[$type];
    }

    function type( $selected)
    {
        $list = explode( ':', $selected);       // event:param:account or action:param:account

        if ( substr( $selected, -7) == 'account') {
            return OCS_PARAM_TYPE_ACCOUNT_KEY;
        }
        else if ( substr( $selected, -4) == 'enum') {
            return OCS_PARAM_TYPE_ENUM;
        }
        else if ( isset( $this->params[$list[1]])) {
            return $this->params[$list[1]];
        }

        return OCS_PARAM_TYPE_STRING;
    }

    function get_form( $param, $operator = NULL)
    {
        $type = $this->type( $param);
        $title = isset( $this->params[$param.':title']) ? $this->params[$param.':title'] : 'Value';

        if ($type == OCS_PARAM_TYPE_ENUM) {
            $options = array_flip( $this->params[$param.':enum']);
            dpm( $options, 'enum');
            $form = array(
                '#type' => ($operator == 'in list') ? 'checkboxes' : 'radios',
                '#title' => $title,
                '#options' => $options,
            );
        }
        else if ( $type == OCS_PARAM_TYPE_COUNTER) {
            $options = ocs_admin_get_counter_id_list();
            $form = array(
                '#type' => 'select',
                '#title' => $title,
                '#options' => array_flip( $options),
            );
        }
        else if ( $type == OCS_PARAM_TYPE_MESSAGE) {
            $options = ocs_admin_get_message_list();
            $form = array(
                '#type' => 'select',
                '#title' => $title,
                '#options' => $options,
            );
        }
        else {
            $form = array(
                '#type' => 'textfield',
                '#title' => $title,
                '#size' => 30,
            );
        }

        return $form;
    }

    function get_value( $param, $value)
    {
        $type = $this->type( $param);
        if ( $type == OCS_PARAM_TYPE_ENUM) {
            $enum_list = array_flip( $this->params[$param.':enum']);

            $selected = array();
            if ( is_array( $value)) {
                foreach( $value as $key => $item) {
                    if ( ! empty($item) ) {  // it's selected
                        $selected[$enum_list[$key]] = $key;
                    }
                }
            }
            else {
                if ( ! isset( $enum_list[$value])) return NULL;
                $selected = array( $enum_list[$value] => $value);
            }

            // return json encoded string
            return array( json_encode( $selected), '('.implode( ', ', array_values( $selected)). ')');
        }

        return array( $value, $value);
    }

    function get_param( $value)
    {
        return isset( $this->params[$value]) ? $this->params[$value] : NULL;
    }

}  
