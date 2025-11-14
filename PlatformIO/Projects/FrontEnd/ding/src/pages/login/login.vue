<template>
	<view class="content">
		<view class="title">登录</view>
		<input class="input" v-model="username" placeholder="用户名：admin" />
		<input class="input" v-model="password" type="password" placeholder="密码：123456" />

		<button class="btn" @click="handleLogin">登 录</button>
	</view>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import indexReq from '@/api/index';

const username = ref('');
const password = ref('');

const handleLogin = async () => {
	const data = {
		username: username.value,
		password: password.value
	};
	await indexReq
		.postLogin(data)
		.then((res: any) => {
			if(res.success == true){
				uni.setStorageSync('token', res.token);
				uni.showToast({ title: '登录成功', icon: 'success' });
				// 跳转页面
				uni.reLaunch({
					url: '/pages/index/index' // 登录成功后跳转首页
				});
			}else{
				uni.showToast({ title: '密码错误', icon: 'fail' });
			}
			
		})
		.catch((err: any) => {
			console.log(err);
			uni.showToast({ title: "网络错误", icon: "none" });
		});
};
</script>

<style lang="scss" scoped></style>
